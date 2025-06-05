// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/regalloc.hpp"
#include "mathsexpr/op.hpp"
#include "mathsexpr/log.hpp"

#include <ranges>
#include <algorithm>

MATHSEXPR_NAMESPACE_BEGIN

void Register::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                case Platform_Linux:
                    std::format_to(std::back_inserter(out), "xmm{}", this->_id);
                    break;
            }
        }
    }
}

void Register::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                case Platform_Linux:
                    break;
            }
        }
    }
}

void Stack::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                case Platform_Linux:
                    std::format_to(std::back_inserter(out), "[rbp - {}]", this->_offset);
                    break;
            }

            break;
        }
    }
}

void Stack::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                case Platform_Linux:
                    break;
            }
        }
    }
}

void Memory::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                case Platform_Linux:
                {
                    if(this->_base_ptr == MemLocRegister_Variables)
                    {
                        std::format_to(std::back_inserter(out),
                                       "[{} + {}]", 
                                       gp_register_x86_64_as_string(get_base_ptr_variable_register(platform, isa)),
                                       this->_offset);
                    }
                    else if(this->_base_ptr == MemLocRegister_Literals)
                    {
                        std::format_to(std::back_inserter(out),
                                       "[{} + {}]", 
                                       gp_register_x86_64_as_string(get_base_ptr_literal_register(platform, isa)),
                                       this->_offset);
                    }

                    break;
                }
            }
        }
    }
}

void Memory::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                case Platform_Linux:
                    break;
            }
        }
    }
}

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

/* Should return a valid register id if we can reuse a register to store the result */
uint64_t ssa_statement_get_reusable_register(const SSAStmtPtr& statement) noexcept
{
    switch(statement->type_id())
    {
        case SSAStmtTypeId_Variable:
        case SSAStmtTypeId_Literal:
            return statement->get_register();
        case SSAStmtTypeId_UnOp:
        {
            auto unop = statement_const_cast<SSAStmtUnOp>(statement.get());
            return unop->get_operand()->get_register();
        }
        case SSAStmtTypeId_BinOp:
        {
            auto binop = statement_const_cast<SSAStmtBinOp>(statement.get());
            return binop->get_left()->get_register();
        }
        case SSAStmtTypeId_FuncOp:
        {
            auto funcop = statement_const_cast<SSAStmtFunctionOp>(statement.get());

            if(funcop->get_arguments().size() == 0)
            {
                return INVALID_STMT_REGISTER;
            }

            return funcop->get_arguments()[0]->get_register();
        }

        default:
            return INVALID_STMT_REGISTER;
    }
}

uint64_t RegisterAllocator::get_max_available_registers(Platform platform, ISA isa) noexcept
{
    switch(platform)
    {
        case Platform_Windows:
        {
            switch(isa)
            {
                case ISA_x86_64:
                    return 5;
                default:
                    return 5;
            }
        }
        case Platform_Linux:
        {
            switch(isa)
            {
                case ISA_x86_64:
                    return 8;
                default:
                    return 8;
            }
        }
        default:
            return 0;
    }
}

using RegisterId = uint64_t;
using StackOffset = uint64_t;
using Active = std::pair<SSAStmtPtr, RegisterId>;

Active select_spill_candidate(std::vector<Active>& candidates) noexcept
{
    Active candidate = std::move(candidates[0]);

    candidates.erase(candidates.begin());

    return candidate;
}

bool RegisterAllocator::allocate(std::vector<SSAStmtPtr>& statements) noexcept
{
    this->_mapping.clear();

    std::vector<SSAStmtPtr> statements_sorted(statements.size());

    std::copy(statements.begin(), statements.end(), statements_sorted.begin());

    std::sort(statements_sorted.begin(), statements_sorted.end(), [](const SSAStmtPtr& a, const SSAStmtPtr& b) -> bool {
        return a->get_live_range().start < b->get_live_range().start;
    });

    std::vector<Active> actives;

    /* To handle spill/load */
    std::unordered_map<SSAStmtPtr, SSAStmtPtr> spilled;
    StackOffset stack_offset = 0;
    uint64_t num_insertions = 0;

    for(size_t i = 0; i < statements.size(); i++)
    {
        auto& stmt = statements[i];

        /* Remove expired intervals from the actives, freeing registers */
        auto remove_result = std::remove_if(actives.begin(), actives.end(), [&](const Active& active) -> bool {
            return active.first->get_live_range().end < stmt->get_live_range().start;
        });

        actives.erase(remove_result, actives.end());

        /* Check if we can reuse an operand register */
        RegisterId reusable_register = ssa_statement_get_reusable_register(stmt);

        if(reusable_register != INVALID_STMT_REGISTER)
        {
            this->_mapping[stmt] = std::make_shared<Register>(reusable_register);
            actives.emplace_back(stmt, reusable_register);
            continue;
        }

        BitVector used_registers;

        for(const auto& [_, reg] : actives)
        {
            used_registers.set(reg);
        }

        RegisterId available_register = used_registers.ffz();

        /* Handle spilling */
        if(available_register > this->_max_registers)
        {
            auto [to_spill, free_reg] = select_spill_candidate(actives);

            SSAStmtPtr spill_stmt = std::make_shared<SSAStmtSpillOp>(to_spill);
            this->_mapping[spill_stmt] = std::make_shared<Stack>(stack_offset);
            stack_offset += 8;

            spilled[to_spill] = spill_stmt;

            statements[i + num_insertions] = spill_stmt;

            num_insertions++;
            i++;

            available_register = free_reg;
        }

        auto insert_load = [&](SSAStmtPtr spill_stmt) -> bool {
            MemLocPtr memloc = this->_mapping[spill_stmt];

            auto stack = memloc_cast<Stack>(memloc.get());

            if(stack == nullptr)
            {
                log_error("Internal error during register allocation: expected stack address, got:");
                // TODO: memloc->print();
                return false;
            }

            SSAStmtPtr load_stmt = std::make_shared<SSAStmtLoadOp>();

            statements[i + num_insertions] = load_stmt;

            num_insertions++;

            return true;
        };

        /* Handle loading */
        switch (stmt->type_id())
        {
            case SSAStmtTypeId_UnOp:
            {
                auto unop = statement_const_cast<SSAStmtUnOp>(stmt.get());

                if(unop == nullptr)
                {
                    log_error("Internal error during register allocation: expected unary op stmt, got:");
                    // stmt->print();
                    return false;
                }

                auto it = spilled.find(unop->get_operand());

                if(it != spilled.end())
                {
                    SSAStmtPtr spill_stmt = it->second;

                    if(!insert_load(spill_stmt))
                    {
                        return false;
                    }

                    spilled.erase(it);
                }

                break;
            }
            case SSAStmtTypeId_BinOp:
            {
                auto binop = statement_cast<SSAStmtBinOp>(stmt.get());

                if(binop == nullptr)
                {
                    log_error("Internal error during register allocation: expected binary op stmt, got:");

                    return false;
                }

                auto left_it = spilled.find(binop->get_left());
                auto right_it = spilled.find(binop->get_right());

                if(left_it != spilled.end())
                {
                    /* to avoid a load, we can invert operands and loadi the left operand */
                    if(op_binary_is_commutative(binop->get_op()) &&
                       right_it == spilled.end())
                    {
                        binop->swap_operands();
                    }
                    else
                    {
                        SSAStmtPtr spill_stmt = left_it->second;

                        if(!insert_load(spill_stmt))
                        {
                            return false;
                        }

                        spilled.erase(left_it);
                    }
                }

                /* If the right operand has been spilled, we can let it on the stack */
                if(right_it != spilled.end())
                {
                    binop->set_right(right_it->second);
                }
            }
            case SSAStmtTypeId_FuncOp:
            {
                break;
            }
            
            default:
                break;
        }

        this->_mapping[stmt] = std::make_shared<Register>(available_register);

        actives.emplace_back(stmt, available_register);
    }

    return true;
}

MATHSEXPR_NAMESPACE_END