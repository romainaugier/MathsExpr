// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/ssa.h"
#include "mathsexpr/op.h"

#include <ranges>
#include <unordered_map>
#include <algorithm>

MATHSEXPR_NAMESPACE_BEGIN

void SSAStmtVariable::print(std::ostream_iterator<char>& out) const noexcept
{
    if(this->get_register() != INVALID_REGISTER)
    {
        std::format_to(out, "{}{} = load {} ({}->{})\n", 
                       REGISTER_CHAR, 
                       this->get_register(),
                       this->_name,
                       this->get_live_range().start,
                       this->get_live_range().end);

    }
    else
    {
        std::format_to(out, "{}{} = load {} ({}->{})\n", 
                       VERSION_CHAR, 
                       this->get_version(),
                       this->_name,
                       this->get_live_range().start,
                       this->get_live_range().end);
    }
}

void SSAStmtLiteral::print(std::ostream_iterator<char>& out) const noexcept
{
    if(this->get_register() != INVALID_REGISTER)
    {
        std::format_to(out, 
                       "{}{} = loadi {} ({}->{})\n",
                       REGISTER_CHAR,
                       this->get_register(),
                       this->_name,
                       this->get_live_range().start,
                       this->get_live_range().end);
    }
    else
    {
        std::format_to(out, 
                       "{}{} = loadi {} ({}->{})\n",
                       VERSION_CHAR,
                       this->get_version(),
                       this->_name,
                       this->get_live_range().start,
                       this->get_live_range().end);
    }
}

void SSAStmtUnOp::print(std::ostream_iterator<char>& out) const noexcept
{
    if(this->get_register() != INVALID_REGISTER)
    {
        std::format_to(out, 
                       "{}{} = {}{}{} ({}->{})\n",
                       REGISTER_CHAR,
                       this->get_register(),
                       op_unary_to_string(this->_op),
                       REGISTER_CHAR,
                       this->_operand->get_register(),
                       this->get_live_range().start,
                       this->get_live_range().end);
    }
    else
    {
        std::format_to(out, 
                       "{}{} = {}{}{} ({}->{})\n",
                       VERSION_CHAR,
                       this->get_version(),
                       op_unary_to_string(this->_op),
                       VERSION_CHAR,
                       this->_operand->get_version(),
                       this->get_live_range().start,
                       this->get_live_range().end);
    }
}

void SSAStmtBinOp::print(std::ostream_iterator<char>& out) const noexcept
{
    if(this->get_register() != INVALID_REGISTER)
    {
        std::format_to(out, 
                       "{}{} = {}{} {} {}{} ({}->{})\n",
                       REGISTER_CHAR,
                       this->get_register(),
                       REGISTER_CHAR,
                       this->_left->get_register(),
                       op_binary_to_string(this->_op),
                       REGISTER_CHAR,
                       this->_right->get_register(),
                       this->get_live_range().start,
                       this->get_live_range().end);
    }
    else
    {
        std::format_to(out, 
                       "{}{} = {}{} {} {}{} ({}->{})\n",
                       VERSION_CHAR,
                       this->get_version(),
                       VERSION_CHAR,
                       this->_left->get_version(),
                       op_binary_to_string(this->_op),
                       VERSION_CHAR,
                       this->_right->get_version(),
                       this->get_live_range().start,
                       this->get_live_range().end);
    }
}

void SSAStmtFunctionOp::print(std::ostream_iterator<char>& out) const noexcept
{
    if(this->get_register() != INVALID_REGISTER)
    {
        std::string arguments;

        for(const auto [i, arg] : std::views::enumerate(this->_arguments))
        {
            std::format_to(std::back_inserter(arguments), 
                           "{}{}{}",
                           i == 0 ? "" : ", ",
                           REGISTER_CHAR,
                           arg->get_register());
        }

        std::format_to(out,
                       "{}{} = {}({}) ({}->{})\n", 
                       REGISTER_CHAR,
                       this->get_register(),
                       this->_name,
                       arguments,
                       this->get_live_range().start,
                       this->get_live_range().end);
    }
    else
    {
        std::string arguments;

        for(const auto [i, arg] : std::views::enumerate(this->_arguments))
        {
            std::format_to(std::back_inserter(arguments), 
                           "{}{}{}",
                           i == 0 ? "" : ", ",
                           VERSION_CHAR,
                           arg->get_version());
        }

        std::format_to(out,
                       "{}{} = {}({}) ({}->{})\n", 
                       VERSION_CHAR,
                       this->get_version(),
                       this->_name,
                       arguments,
                       this->get_live_range().start,
                       this->get_live_range().end);
    }
}

void SSAStmtAllocateStackOp::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out, "stackalloc ({} bytes)\n", this->_size);
}

void SSAStmtSpillOp::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out, "spill {} [sp - {}]\n", this->_operand, this->_offset);
}

void SSAStmtLoadOp::print(std::ostream_iterator<char>& out) const noexcept
{
    if(this->get_register() != INVALID_REGISTER)
    {
        std::format_to(out, "load {}{} [sp - {}]\n", REGISTER_CHAR, this->get_register(), this->_offset);
    }
    else
    {
        std::format_to(out, "load {}{} [sp - {}]\n", VERSION_CHAR, this->get_version(), this->_offset);
    }
}

void SSA::print() const noexcept
{
    static std::ostream_iterator<char> out(std::cout);

    std::format_to(out, "SSA\n");

    for(const auto& stmt : this->_statements)
    {
        stmt->print(out);
    }
}

uint64_t SSAStmtVariable::canonicalize() const noexcept
{
    return 0;
}

uint64_t SSAStmtLiteral::canonicalize() const noexcept
{
    return 0;
}

uint64_t SSAStmtUnOp::canonicalize() const noexcept
{
    return 0;
}

uint64_t SSAStmtBinOp::canonicalize() const noexcept
{
    return 0;
}

uint64_t SSAStmtFunctionOp::canonicalize() const noexcept
{
    return 0;
}

uint64_t SSAStmtAllocateStackOp::canonicalize() const noexcept
{
    return 0;
}

uint64_t SSAStmtSpillOp::canonicalize() const noexcept
{
    return 0;
}

uint64_t SSAStmtLoadOp::canonicalize() const noexcept
{
    return 0;
}

/* SSA */

bool SSA::build_from_ast(const AST& ast) noexcept
{
    this->_statements.clear();
    uint64_t version = 0;
    bool no_error = true;

    std::unordered_map<const ASTNode*, SSAStmtPtr> mapping;

    auto traverse = [&](auto&& self, const ASTNode* node) {
        if(node == nullptr)
        {
            return;
        }

        switch(node->type_id())
        {
            case ASTNodeTypeId_Variable:
            {
                const ASTNodeVariable* variable_node = node_cast<ASTNodeVariable>(node);

                SSAStmtPtr variable = std::make_shared<SSAStmtVariable>(variable_node->get_name(),
                                                                        version++,
                                                                        this->get_statement_number());

                this->_statements.push_back(variable);
                mapping[variable_node] = variable;

                break;
            }
            case ASTNodeTypeId_Literal:
            {
                const ASTNodeLiteral* literal_node = node_cast<ASTNodeLiteral>(node);

                SSAStmtPtr literal = std::make_shared<SSAStmtLiteral>(literal_node->get_name(),
                                                                      version++,
                                                                      this->get_statement_number());

                this->_statements.push_back(literal);
                mapping[literal_node] = literal;

                break;
            }
            case ASTNodeTypeId_UnOp:
            {
                const ASTNodeUnaryOp* unop_node = node_cast<ASTNodeUnaryOp>(node);

                self(self, unop_node->get_operand());

                if(!mapping.contains(unop_node->get_operand()))
                {
                    no_error = false;
                    // TODO log_error();
                    return;
                }

                mapping[unop_node->get_operand()]->get_live_range().end = this->get_statement_number();

                SSAStmtPtr unop = std::make_shared<SSAStmtUnOp>(mapping[unop_node->get_operand()],
                                                                unop_node->get_op(),
                                                                version++,
                                                                this->get_statement_number());

                this->_statements.push_back(unop);
                mapping[unop_node] = unop;

                break;
            }
            case ASTNodeTypeId_BinOp:
            {
                const ASTNodeBinaryOp* binop_node = node_cast<ASTNodeBinaryOp>(node);

                self(self, binop_node->get_left());
                self(self, binop_node->get_right());

                if(!mapping.contains(binop_node->get_left()) && !mapping.contains(binop_node->get_right()))
                {
                    no_error = false;
                    return;
                }

                mapping[binop_node->get_left()]->get_live_range().end = this->get_statement_number();
                mapping[binop_node->get_right()]->get_live_range().end = this->get_statement_number();

                SSAStmtPtr binop = std::make_shared<SSAStmtBinOp>(mapping[binop_node->get_left()],
                                                                  mapping[binop_node->get_right()],
                                                                  binop_node->get_op(),
                                                                  version++,
                                                                  this->get_statement_number());

                this->_statements.push_back(binop);
                mapping[binop_node] = binop;

                break;
            }
            case ASTNodeTypeId_FuncOp:
            {
                const ASTNodeFunctionOp* funccall_node = node_cast<ASTNodeFunctionOp>(node);

                for(const auto& argument : funccall_node->get_arguments())
                {
                    self(self, argument.get());
                }

                std::vector<SSAStmtPtr> arguments;

                for(const auto& argument : funccall_node->get_arguments())
                {
                    if(!mapping.contains(argument.get()))
                    {
                        no_error = false;
                        return;
                    }

                    arguments.push_back(mapping[argument.get()]);
                }

                for(auto& argument : arguments)
                {
                    argument->get_live_range().end = this->get_statement_number();
                }

                SSAStmtPtr funccall = std::make_shared<SSAStmtFunctionOp>(funccall_node->get_function_name(),
                                                                          std::move(arguments),
                                                                          version++,
                                                                          this->get_statement_number());

                this->_statements.push_back(funccall);
                mapping[funccall_node] = funccall;
            }
            
            default:
                break;
        }
    };

    traverse(traverse, ast.get_root());

    return no_error;
}

/* SSA Register allocation */

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

    size_t ffz() const noexcept
    {
        size_t first = 0;

        while(first < (SIZE * BIT_SIZE) && this->get(first))
        {
            first++;
        }

        return first;
    }


    size_t ffs() const noexcept
    {
        size_t first = 0;

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

    void operator&(const BitVector& other) noexcept
    {
        for(size_t i = 0; i < SIZE; i++)
        {
            this->_data[i] &= other._data[i];
        }
    }

    void operator|(const BitVector& other) noexcept
    {
        for(size_t i = 0; i < SIZE; i++)
        {
            this->_data[i] |= other._data[i];
        }
    }

    void operator^(const BitVector& other) noexcept
    {
        for(size_t i = 0; i < SIZE; i++)
        {
            this->_data[i] ^= other._data[i];
        }
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

using Active = std::tuple<SSAStmtPtr, BitVector>;

bool SSA::allocate_registers(Platform platform, ISA isa) noexcept
{
    uint64_t max_registers = platform == Platform_Linux ? 8 : 5;
    uint64_t return_register = 0; /* xmm0 */

    /* variable, register */
    std::vector<Active> actives;

    std::vector<SSAStmtPtr> statements = this->_statements;
    std::ranges::sort(statements, [](const SSAStmtPtr& a, const SSAStmtPtr& b) -> bool {
        return a->get_live_range().start < b->get_live_range().start;
    });

    for(auto& stmt : statements)
    {
        /* Remove the expired intervals */
        auto result = std::remove_if(actives.begin(), actives.end(), [&](const Active& active) -> bool {
            return std::get<0>(active)->get_live_range().end <= stmt->get_live_range().start;
        });

        actives.erase(result, actives.end());

        BitVector used_regs;

        for(const auto [_, regs] : actives)
        {
            used_regs.operator|(regs);
        }

        uint64_t reg = used_regs.ffz();
        stmt->set_register(reg);
        used_regs.set(reg);

        actives.emplace_back(stmt, used_regs);
    }

    return true;
}

MATHSEXPR_NAMESPACE_END