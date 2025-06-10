// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/regalloc.hpp"
#include "mathsexpr/op.hpp"
#include "mathsexpr/log.hpp"

#include <ranges>
#include <algorithm>
#include <cstring>
#include <unordered_set>

MATHSEXPR_NAMESPACE_BEGIN

/* Register allocation on SSA */

/*
    Since we don't have phi-nodes in our SSA form, register allocation is trivial and can be performed
    with a linear scan (with constraints). Since we only support floating point operations, we only
    allocate in fp registers (xmm[i])

    For Linux x86_64, we can use xmm0-xmm7
    For Windows x86_64, we can use xmm0-xmm5
*/

/* 
    Simple helper structure. For now, since we won't allocate on more than 64 registers, we can
    assume that a 64 bits integer will be sufficient. The structure can be adapted in the future 
    to hold more registers
*/

class BitVector 
{
    static constexpr size_t SIZE = 1;
    static constexpr size_t BIT_SIZE = 64;
    static constexpr size_t BYTE_SIZE = sizeof(uint64_t);

private:
    uint64_t _data[SIZE];

    std::tuple<size_t, size_t> get_index(size_t index) const noexcept
    {
        return std::make_tuple(index / BIT_SIZE, index % BIT_SIZE);
    }

public:
    BitVector() 
    {
        this->reset();
    }

    BitVector(const BitVector& other)
    {
        std::memcpy(this->_data, other._data, SIZE * BYTE_SIZE);
    }

    BitVector(BitVector&& other) noexcept
    {
        std::memcpy(this->_data, other._data, SIZE * BYTE_SIZE);
    }

    BitVector operator=(const BitVector& other)
    {
        std::memcpy(this->_data, other._data, SIZE * BYTE_SIZE);

        return *this;
    }

    BitVector operator=(BitVector&& other) noexcept
    {
        std::memcpy(this->_data, other._data, SIZE * BYTE_SIZE);

        return *this;
    }

    bool get(size_t index) const noexcept
    {
        MATHSEXPR_ASSERT(index < (SIZE * BIT_SIZE), "Out-of-bounds access");

        const auto [arr_index, bit_index]= this->get_index(index);

        return (_data[arr_index] >> bit_index) & 1;
    }

    void set(size_t index) 
    {
        MATHSEXPR_ASSERT(index < (SIZE * BIT_SIZE), "Out-of-bounds access");

        const auto [arr_index, bit_index]= this->get_index(index);

        this->_data[arr_index] |= (uint64_t(1) << bit_index);
    }

    void clear(size_t index) noexcept
    {
        MATHSEXPR_ASSERT(index < (SIZE * BIT_SIZE), "Out-of-bounds access");

        const auto [arr_index, bit_index]= this->get_index(index);

        this->_data[arr_index] &= ~(uint64_t(1) << bit_index);
    }

    /* Find first zero */
    size_t ffz() const noexcept
    {
        size_t first = 0;

        /* TODO: use __builtin_clz/BitScanForward per qword */
        while(first < (SIZE * BIT_SIZE) && this->get(first))
        {
            first++;
        }

        return first;
    }

    /* Find first set (1) */
    size_t ffs() const noexcept
    {
        size_t first = 0;

        /* TODO: use __builtin_ffs/BitScanReverse per qword */
        while(first < (SIZE * BIT_SIZE) && !this->get(first))
        {
            first++;
        }

        return first;
    }

    void reset() noexcept
    {
        std::memset(this->_data, 0, SIZE * BYTE_SIZE);
    }

    void print() const noexcept
    {
        static std::ostream_iterator<char> out(std::cout);

        for(size_t i = 0; i < SIZE * BIT_SIZE; i++)
        {
            std::format_to(out, "{}", this->get(i) ? '1' : '0');
        }

        std::format_to(out, "\n");
    }
};

/* Utilities */

/* Should return a valid register if we can reuse a register to store the result */
const MemLocPtr RegisterAllocator::get_reusable_register(const SSAStmtPtr& statement) const noexcept
{
    switch(statement->type_id())
    {
        case SSAStmtTypeId_Variable:
        case SSAStmtTypeId_Literal:
        {
            auto it = this->_mapping.find(statement);

            return it == this->_mapping.end() ? nullptr : it->second;
        }
        case SSAStmtTypeId_UnOp:
        {
            auto unop = statement_const_cast<SSAStmtUnOp>(statement.get());

            auto it = this->_mapping.find(unop->get_operand());

            return it == this->_mapping.end() ? nullptr : it->second;
        }
        case SSAStmtTypeId_BinOp:
        {
            auto binop = statement_const_cast<SSAStmtBinOp>(statement.get());

            auto it = this->_mapping.find(binop->get_left());

            return it == this->_mapping.end() ? nullptr : it->second;
        }
        case SSAStmtTypeId_FuncOp:
        {
            auto funcop = statement_const_cast<SSAStmtFunctionOp>(statement.get());

            if(funcop->get_arguments().size() == 0)
            {
                return nullptr;
            }

            auto it = this->_mapping.find(funcop->get_arguments()[0]);

            return it == this->_mapping.end() ? nullptr : it->second;
        }

        default:
            return nullptr;
    }
}

/* Optimization passes for better register allocation */

/*
    This pass swaps operands of commutative binary ops if the right operand can be loaded from the 
    stack or constant-memory (i.e a literal, a variable, a spilled temporary)
*/
bool RegisterAllocator::prepass_commutative_operand_swap(SSA& ssa) noexcept
{
    for(auto& stmt : ssa.get_statements())
    {
        switch(stmt->type_id())
        {
            case SSAStmtTypeId_BinOp:
            {
                auto binop = statement_cast<SSAStmtBinOp>(stmt.get());

                if(binop == nullptr)
                {
                    log_error("Error during commutative operand swap opt pass. Expected binop, got: {}",
                              stmt->type_id());
                    return false;
                }

                if(!op_binary_is_commutative(binop->get_op()))
                {
                    break;
                }

                auto& left = binop->get_left();
                auto& right = binop->get_right();

                if(left->type_id() == SSAStmtTypeId_Literal &&
                   right->type_id() != SSAStmtTypeId_Literal)
                {
                    binop->swap_operands();
                }

                if(left->type_id() == SSAStmtTypeId_Variable && 
                   right->type_id() != SSAStmtTypeId_Variable)
                {
                    binop->swap_operands();
                }

                if(left->type_id() == SSAStmtTypeId_SpillOp &&
                   right->type_id() != SSAStmtTypeId_SpillOp)
                {
                    binop->swap_operands();
                }

                break;
            }
        }
    }

    return true;
}

/* Register allocation */

using StackOffset = uint64_t;
using Active = std::pair<SSAStmtPtr, RegisterId>;

Active select_spill_candidate(std::vector<Active>& candidates) noexcept
{
    uint64_t duration = 0;
    size_t position = 0;

    for(auto [i, candidate] : std::ranges::enumerate_view(candidates))
    {
        if(candidate.first->get_live_range().get_duration() > duration)
        {
            duration = candidate.first->get_live_range().get_duration();
            position = i;
        }
    }

    Active candidate = std::move(candidates[position]);

    candidates.erase(candidates.begin() + position);

    return candidate;
}

bool RegisterAllocator::allocate(SSA& ssa,
                                 const SymbolTable& symtable) noexcept
{
    /* multi-pass register allocation using linear scan. we iteratively insert spills/loads */
    uint32_t num_passes = 0;
    uint32_t max_pressure = 0;
    uint64_t needed_stack_size = 0;

    std::unordered_map<SSAStmtPtr, SSAStmtPtr> spilled;

    while(true)
    {
        num_passes++;

        log_debug("Register allocation: pass {}", num_passes);

        needed_stack_size = 0;
        this->_mapping.clear();

        if(!ssa.calculate_live_ranges())
        {
            return false;
        }

        if(!RegisterAllocator::prepass_commutative_operand_swap(ssa))
        {
            return false;
        }

        std::vector<Active> actives;

        std::vector<SSAStmtPtr>& statements = ssa.get_statements();

        /* 
            Allocation of constrained ops: - function calls return in xmm0
                                           - expr return value in xmm0
                                           - function args must go in xmm[i]
                                           - allocate the memory address of each literal
        
        */

        /* We only deal with fp values (double or float) so we only care about this rv */
        RegisterId rv_reg = get_call_return_value_fp_register(this->_platform, this->_isa);

        SSAStmtPtr& last_stmt = statements.back();

        this->_mapping[last_stmt] = std::make_shared<Register>(rv_reg);
        actives.emplace_back(last_stmt, rv_reg);

        switch(last_stmt->type_id())
        {
            case SSAStmtTypeId_UnOp:
            {
                auto unop = statement_cast<SSAStmtUnOp>(last_stmt.get());

                if(unop == nullptr)
                {
                    log_error("Error during return value register allocation. Expected unop, got: {}",
                            last_stmt->type_id());
                    return false;
                }

                auto operand = unop->get_operand();

                this->_mapping[operand] = std::make_shared<Register>(rv_reg);
                actives.emplace_back(operand, rv_reg);

                break;
            }

            case SSAStmtTypeId_BinOp:
            {
                auto binop = statement_cast<SSAStmtBinOp>(last_stmt.get());

                if(binop == nullptr)
                {
                    log_error("Error during return value register allocation. Expected binop, got: {}",
                            last_stmt->type_id());
                    return false;
                }

                auto left = binop->get_left();

                this->_mapping[left] = std::make_shared<Register>(rv_reg);
                actives.emplace_back(left, rv_reg);

                break;
            }
        }

        for(int64_t i = statements.size() - 2; i >= 0; i--)
        {
            auto& stmt = statements[i];

            switch(stmt->type_id())
            {
                case SSAStmtTypeId_Literal:
                {
                    auto literal = statement_cast<SSAStmtLiteral>(stmt.get());

                    if(literal == nullptr)
                    {
                        log_error("Internal error during register allocation. Expected literal, got: {}",
                                  stmt->type_id());

                        return false;
                    }

                    if(this->_mapping.contains(stmt))
                    {
                        break;
                    }

                    this->_mapping[stmt] = std::make_shared<Memory>(MemLocRegister_Literals,
                                                                    symtable.get_literal_offset(literal->get_name()));

                    break;
                }

                case SSAStmtTypeId_Variable:
                {
                    auto variable = statement_cast<SSAStmtVariable>(stmt.get());

                    if(variable == nullptr)
                    {
                        log_error("Internal error during register allocation. Expected variable, got: {}",
                                  stmt->type_id());

                        return false;
                    }

                    if(this->_mapping.contains(stmt))
                    {
                        break;
                    }

                    this->_mapping[stmt] = std::make_shared<Memory>(MemLocRegister_Variables,
                                                                    symtable.get_variable_offset(variable->get_name()));

                    break;
                }

                case SSAStmtTypeId_BinOp:
                {
                    auto binop = statement_cast<SSAStmtBinOp>(stmt.get());

                    if(binop == nullptr)
                    {
                        log_error("Internal error during register allocation. Expected binop, got: {}",
                                  stmt->type_id());

                        return false;
                    }

                    if(this->_mapping.contains(stmt))
                    {
                        RegisterId reg = memloc_cast<Register>(this->_mapping[stmt].get())->get_id();

                        this->_mapping[binop->get_left()] = std::make_shared<Register>(reg);
                        actives.emplace_back(binop->get_left(), reg);
                    }

                    break;
                }

                case SSAStmtTypeId_FuncOp:
                {
                    RegisterId return_value_register = get_call_return_value_fp_register(this->_platform, 
                                                                                         this->_isa);

                    if(return_value_register == INVALID_FP_REGISTER)
                    {
                        log_error("Error during register allocation, check the log for more information");
                        return false;
                    }

                    this->_mapping[stmt] = std::make_shared<Register>(return_value_register);
                    actives.emplace_back(stmt, return_value_register);

                    auto funcop = statement_cast<SSAStmtFunctionOp>(stmt.get());

                    if(funcop == nullptr)
                    {
                        log_error("Internal error during register allocation. Expected func op, got: {}", 
                                stmt->type_id());
                        return false;
                    }

                    if(funcop->get_arguments().size() > get_call_max_args_fp_registers(this->_platform,
                                                                                       this->_isa))
                    {
                        return false;
                    }

                    auto& args_registers = get_call_args_fp_registers(this->_platform,
                                                                      this->_isa);

                    for(auto [i, argument] : std::ranges::enumerate_view(funcop->get_arguments()))
                    {
                        this->_mapping[argument] = std::make_shared<Register>(args_registers[i]);
                        actives.emplace_back(argument, args_registers[i]);
                    }

                    break;
                }
            }
        }

        /* Allocation of non-constrained ops */

        std::vector<SSAStmtPtr> statements_sorted(statements.size());

        std::copy(statements.begin(), statements.end(), statements_sorted.begin());

        std::sort(statements_sorted.begin(),
                  statements_sorted.end(),
                  [](const SSAStmtPtr& a, const SSAStmtPtr& b) -> bool {
            return a->get_live_range().start < b->get_live_range().start;
        });

        std::unordered_set<SSAStmtPtr> to_spill;
        uint64_t stack_offset = 0;

        std::unordered_set<SSAStmtPtr> to_load;

        for(size_t i = 0; i < statements_sorted.size(); i++)
        {
            auto& stmt = statements_sorted[i];

            /* Remove expired intervals from the actives, freeing registers */
            auto remove_result = std::remove_if(actives.begin(), 
                                                actives.end(),
                                                [&](const Active& active) -> bool {
                return active.first->get_live_range().end < stmt->get_live_range().start;
            });

            actives.erase(remove_result, actives.end());

            /*  
                Check if we have to insert a load. Since we let all literals and variables in memory, 
                if we need a register for one, load it here 
                TODO: maybe check function ops too
            */
            switch(stmt->type_id())
            {
                case SSAStmtTypeId_UnOp:
                {
                    auto unop = statement_cast<SSAStmtUnOp>(stmt.get());

                    if(unop == nullptr)
                    {
                        log_error("Internal error during register allocation. Expected unop, got: {}", 
                                  stmt->type_id());
                        return false;
                    }

                    if(unop->get_operand()->type_id() == SSAStmtTypeId_Literal || 
                       unop->get_operand()->type_id() == SSAStmtTypeId_Variable)
                    {
                        to_load.insert(unop->get_operand());
                    }

                    break;
                }

                case SSAStmtTypeId_BinOp:
                {
                    auto binop = statement_cast<SSAStmtBinOp>(stmt.get());

                    if(binop == nullptr)
                    {
                        log_error("Internal error during register allocation. Expected binop, got: {}",
                                  stmt->type_id());
                        return false;
                    }

                    if(binop->get_left()->type_id() == SSAStmtTypeId_Literal ||
                       binop->get_left()->type_id() == SSAStmtTypeId_Variable)
                    {
                        to_load.insert(binop->get_left());
                    }

                    break;
                }
            }

            if(this->_mapping.contains(stmt))
            {
                continue;
            }

            /* Check if we can reuse a register used in the op to store the result */
            MemLocPtr reusable_register = this->get_reusable_register(stmt);

            if(reusable_register != nullptr &&
               reusable_register->type_id() == MemLocTypeId_Register)
            {
                auto reg = memloc_cast<Register>(reusable_register.get());

                if(reg == nullptr)
                {
                    log_error("Internal error during register allocation. Expected register, got: {}",
                              reusable_register->type_id());
                    return false;
                }

                this->_mapping[stmt] = std::make_shared<Register>(reg->get_id());
                actives.emplace_back(stmt, reg->get_id());
                continue;
            }

            if(stmt->type_id() == SSAStmtTypeId_SpillOp)
            {
                auto spill = statement_cast<SSAStmtSpillOp>(stmt.get());

                this->_mapping[stmt] = std::make_shared<Stack>(stack_offset + 8);
                stack_offset += 8;

                needed_stack_size = std::max(needed_stack_size, stack_offset);

                auto remove_result = std::remove_if(actives.begin(), 
                                                    actives.end(), 
                                                    [&](const Active& active) -> bool {
                    return active.first == spill->get_operand();
                });

                actives.erase(remove_result, actives.end());

                continue;
            }

            BitVector used_registers;

            for(const auto& [_, reg] : actives)
            {
                used_registers.set(reg);
            }

            RegisterId available_register = used_registers.ffz();

            max_pressure = std::max(max_pressure, static_cast<uint32_t>(available_register));

            /* Handle spilling */
            if(available_register >= this->_max_registers)
            {
                auto [stmt_to_spill, free_reg] = select_spill_candidate(actives);

                to_spill.insert(stmt_to_spill);

                available_register = free_reg;
            }

            this->_mapping[stmt] = std::make_shared<Register>(available_register);
            actives.emplace_back(stmt, available_register);
        }

        /* register allocation is successful */
        if(to_spill.size() == 0 && to_load.size() == 0)
        {
            break;
        }

        std::vector<SSAStmtPtr> new_statements;
        new_statements.reserve(statements.size() + to_spill.size() * 2);

        uint64_t version = statements.size();

        for(auto stmt : statements)
        {
            /* Check if we need to add loads */
            switch(stmt->type_id())
            {
                case SSAStmtTypeId_UnOp:
                {
                    auto unop = statement_cast<SSAStmtUnOp>(stmt.get());

                    if(unop == nullptr)
                    {
                        log_error("Error during load op insertion. Expected unop, got: {}",
                                  stmt->type_id());
                        return false;
                    }

                    auto& operand = unop->get_operand();

                    if(to_spill.contains(operand))
                    {
                        if(!spilled.contains(operand))
                        {
                            log_error("Error during spilled search. Cannot find spill statement for op: {}",
                                      operand->get_version());
                            return false;
                        }

                        SSAStmtPtr load = std::make_shared<SSAStmtLoadOp>(spilled[operand], version++);
                        unop->set_operand(load);
                        new_statements.emplace_back(load);

                        log_debug("Inserted load op for ssa var: {}{}",
                                  VERSION_CHAR,
                                  operand->get_version());
                    }

                    if(to_load.contains(operand))
                    {
                        SSAStmtPtr load = std::make_shared<SSAStmtLoadOp>(operand, version++);
                        unop->set_operand(load);
                        new_statements.emplace_back(load);

                        log_debug("Inserted load op for ssa var: {}{}",
                                  VERSION_CHAR,
                                  operand->get_version());
                    }

                    break;
                }

                case SSAStmtTypeId_BinOp:
                {
                    auto binop = statement_cast<SSAStmtBinOp>(stmt.get());

                    if(binop == nullptr)
                    {
                        log_error("Error during load op insertion. Expected binop, got: {}",
                                  stmt->type_id());
                        return false;
                    }

                    auto& left = binop->get_left();
                    auto& right = binop->get_right();

                    auto it = to_spill.find(left);

                    if(it != to_spill.end())
                    {
                        if(!spilled.contains(left))
                        {
                            log_error("Error during spilled search. Cannot find spill statement for op: {}",
                                      left->get_version());
                            return false;
                        }

                        SSAStmtPtr load = std::make_shared<SSAStmtLoadOp>(spilled[left], version++);
                        binop->set_left(load);
                        new_statements.emplace_back(load);

                        log_debug("Inserted load op for ssa var: {}{}",
                                  VERSION_CHAR,
                                  left->get_version());
                    }

                    if(to_load.contains(left))
                    {
                        SSAStmtPtr load = std::make_shared<SSAStmtLoadOp>(left, version++);
                        binop->set_left(load);
                        new_statements.emplace_back(load);

                        log_debug("Inserted load op for ssa var: {}{}", 
                                  VERSION_CHAR,
                                  left->get_version());
                    }

                    /* 
                        We don't need to load the right operand because it can be used in
                        immediate mode
                        We take care of that below, once all registers have been allocated
                    */

                    break;
                }

                case SSAStmtTypeId_FuncOp:
                {
                    auto funcop = statement_cast<SSAStmtFunctionOp>(stmt.get());

                    if(funcop == nullptr)
                    {
                        log_error("Error during load op insertion. Expected funcop, got: {}",
                                  stmt->type_id());
                        return false;
                    }

                    for(auto [i, arg] : std::ranges::enumerate_view(funcop->get_arguments()))
                    {
                        if(to_spill.contains(arg))
                        {
                            SSAStmtPtr load = std::make_shared<SSAStmtLoadOp>(arg, version++);
                            funcop->get_arguments()[i] = load;
                            new_statements.emplace_back(load);

                            log_debug("Inserted load op for ssa var: {}{}", 
                                      VERSION_CHAR,
                                      arg->get_version());
                        }
                    }

                    break;
                }
            }

            new_statements.emplace_back(stmt);

            /* Check if we need to add a spill */
            if(to_spill.contains(stmt))
            {
                SSAStmtPtr spill = std::make_shared<SSAStmtSpillOp>(stmt, version++);
                new_statements.emplace_back(spill);

                spilled[stmt] = spill;

                log_debug("Inserted spill op for ssa var: {}{}", 
                          VERSION_CHAR,
                          stmt->get_version());
            }
        }

        ssa.get_statements() = std::move(new_statements);
    }

    log_debug("Allocated registers in {} pass{} (max pressure: {})", 
              num_passes,
              num_passes > 1 ? "es" : "",
              max_pressure + 1);

    if(needed_stack_size > 0)
    {
        /* Stack needs to be aligned to 16 for sse */
        needed_stack_size = (needed_stack_size + 15) & ~15;

        log_debug("Adding stackalloc op (needed space: {})", needed_stack_size);

        ssa.get_statements().emplace(ssa.get_statements().begin(),
                                     std::make_shared<SSAStmtAllocateStackOp>(needed_stack_size));
    }

    return true;
}

MATHSEXPR_NAMESPACE_END