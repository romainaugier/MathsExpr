// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_CODEGEN)
#define __MATHEXPR_CODEGEN

#include "mathexpr/regalloc.hpp"
#include "mathexpr/symtable.hpp"
#include "mathexpr/bytecode.hpp"
#include "mathexpr/link.hpp"

#include <variant>
#include <functional>

MATHEXPR_NAMESPACE_BEGIN

/* Base abstract instruction that is target agnostic */
class MATHEXPR_API Instr
{
    PlatformABIPtr _platform_abi;

public:
    Instr(PlatformABIPtr platform_abi = nullptr) : _platform_abi(platform_abi) {}

    virtual ~Instr() = default;

    virtual void as_string(std::string& out) const noexcept = 0;
    virtual void as_bytecode(ByteCode& out) const noexcept = 0;

    /* default estimation, useful to avoid reallocation when emitting bytecode */
    virtual size_t get_bytecode_size_estimate() const noexcept { return 4; }

    /* for instructions that needs linking once the code reaches linking phase (i.e, function calls) */
    virtual bool needs_linking() const noexcept { return false; }

    /* for instructions that need linking, return linking information */
    virtual RelocInfo get_link_info(std::size_t bytecode_start) const noexcept { return RelocInfo(); }
};

using InstrPtr = std::shared_ptr<Instr>;

/* Target specific code generator, subclassed in target files (x86_64.cpp, aarch64.cpp ...) */
class MATHEXPR_API TargetCodeGenerator
{
    PlatformABIPtr _platform_abi;

public:
    TargetCodeGenerator(PlatformABIPtr platform_abi) : _platform_abi(platform_abi) {}

    virtual ~TargetCodeGenerator() = default;

    virtual bool is_valid() const noexcept = 0;

    virtual InstrPtr create_mov(MemLocPtr& from, MemLocPtr& to) = 0;
    virtual InstrPtr create_prologue(uint64_t stack_size) = 0;
    virtual InstrPtr create_epilogue(uint64_t stack_size) = 0;
    virtual InstrPtr create_neg(MemLocPtr& operand) = 0;
    virtual InstrPtr create_add(MemLocPtr& left, MemLocPtr& right) = 0;
    virtual InstrPtr create_sub(MemLocPtr& left, MemLocPtr& right) = 0;
    virtual InstrPtr create_mul(MemLocPtr& left, MemLocPtr& right) = 0;
    virtual InstrPtr create_div(MemLocPtr& left, MemLocPtr& right) = 0;
    virtual InstrPtr create_call(std::string_view call_name) = 0;
    virtual InstrPtr create_ret() = 0;

    /* Add more instructions */

    std::string_view get_target_name() const noexcept { return this->_platform_abi->get_as_string(); }

    PlatformABIPtr get_platform_abi() noexcept { return this->_platform_abi; }

    virtual void optimize_instr_sequence(std::vector<InstrPtr>& instructions) {}
};

using TargetCodeGeneratorPtr = std::unique_ptr<TargetCodeGenerator>;

/* Main code generator, target agnostic */
class MATHEXPR_API CodeGenerator
{
private:
    std::vector<InstrPtr> _instructions;
    TargetCodeGeneratorPtr _target_generator;

    uint32_t _isa;
    PlatformABIPtr _platform_abi;

public:
    CodeGenerator(uint32_t isa, PlatformABIPtr platform_abi);

    bool build(const SSA& ssa,
               const RegisterAllocator& regalloc,
               SymbolTable& symtable) noexcept;

    std::tuple<bool, ByteCode> as_bytecode(Relocations& relocs) const noexcept;
    std::tuple<bool, std::string> as_string() const noexcept;
    std::tuple<bool, std::string> as_bytecode_hex_string() const noexcept;

    std::string_view get_target_name() const noexcept { return this->_platform_abi->get_as_string(); }

    void add_instruction(InstrPtr instr) noexcept { this->_instructions.push_back(std::move(instr)); }
    const std::vector<InstrPtr>& get_instructions() const noexcept { return this->_instructions; }

private:
    static TargetCodeGeneratorPtr create_target_generator(uint32_t isa,
                                                          PlatformABIPtr platform_abi) noexcept;
};

/* Target factory registration */
class MATHEXPR_API TargetRegistry
{
public:
    using TargetFactory = std::function<TargetCodeGeneratorPtr(PlatformABIPtr)>;

    static void register_target(uint32_t isa, TargetFactory factory) noexcept;
    static TargetCodeGeneratorPtr create_target(uint32_t isa, PlatformABIPtr platform_abi) noexcept;
    static std::unordered_set<uint32_t> get_supported_isas() noexcept;
    static bool is_supported(uint32_t isa, PlatformABIPtr platform_abi) noexcept;

private:
    static std::unordered_map<uint32_t, TargetFactory>& get_registry() noexcept;
};

#define REGISTER_TARGET(isa_enum, target_class) \
    namespace { \
        static bool _registered_##target_class = []() { \
            TargetRegistry::register_target(isa_enum, \
                [](PlatformABIPtr platform_abi) -> TargetCodeGeneratorPtr { \
                    return std::make_unique<target_class>(platform_abi); \
                }); \
            return true; \
        }(); \
    }

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_CODEGEN) */
