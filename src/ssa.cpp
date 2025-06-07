// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/ssa.hpp"
#include "mathsexpr/op.hpp"
#include "mathsexpr/log.hpp"

#include <ranges>
#include <unordered_map>
#include <algorithm>
#include <format>

MATHSEXPR_NAMESPACE_BEGIN

void SSAStmtVariable::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out, "{}{} = load {} ({}->{})\n", 
                   VERSION_CHAR, 
                   this->get_version(),
                   this->_name,
                   this->get_live_range().start,
                   this->get_live_range().end);
}

void SSAStmtLiteral::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out, 
                   "{}{} = loadi {} ({}->{})\n",
                   VERSION_CHAR,
                   this->get_version(),
                   this->_name,
                   this->get_live_range().start,
                   this->get_live_range().end);
}

void SSAStmtUnOp::print(std::ostream_iterator<char>& out) const noexcept
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

void SSAStmtBinOp::print(std::ostream_iterator<char>& out) const noexcept
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

void SSAStmtFunctionOp::print(std::ostream_iterator<char>& out) const noexcept
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

void SSAStmtAllocateStackOp::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out, "stackalloc ({} bytes)\n", this->_size);
}

void SSAStmtSpillOp::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out, "spill {}{}\n", VERSION_CHAR, this->_operand->get_version());
}

void SSAStmtLoadOp::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out, "{}{} = load\n", VERSION_CHAR, this->get_version());
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

bool SSA::calculate_live_ranges() noexcept
{
    for(auto [i, statement]: std::ranges::enumerate_view(this->_statements))
    {
        statement->get_live_range().start = i;
        statement->get_live_range().end = i + 1;

        switch(statement->type_id())
        {
            case SSAStmtTypeId_UnOp:
            {
                auto unop = statement_cast<SSAStmtUnOp>(statement.get());

                if(unop == nullptr)
                {
                    log_error("Internal error during live ranges calculation. Expected unop, got: {}",
                              statement->type_id());
                    return false;
                }

                unop->get_operand()->get_live_range().set_end(i);

                break;
            }

            case SSAStmtTypeId_BinOp:
            {
                auto binop = statement_cast<SSAStmtBinOp>(statement.get());

                if(binop == nullptr)
                {
                    log_error("Internal error during live ranges calculation. Expected binop, got: {}",
                              statement->type_id());
                    return false;
                }

                binop->get_left()->get_live_range().set_end(i);
                binop->get_right()->get_live_range().set_end(i);

                break;
            }

            case SSAStmtTypeId_FuncOp:
            {
                auto funcop = statement_cast<SSAStmtFunctionOp>(statement.get());

                if(funcop == nullptr)
                {
                    log_error("Internal error during live ranges calculation. Expected funcop, got: {}",
                              statement->type_id());
                    return false;
                }

                for(auto arg : funcop->get_arguments())
                {
                    arg->get_live_range().set_end(i);
                }

                break;
            }

            case SSAStmtTypeId_SpillOp:
            {
                auto spillop = statement_cast<SSAStmtSpillOp>(statement.get());

                if(spillop == nullptr)
                {
                    log_error("Internal error during live ranges calculation. Expected spillop, got: {}",
                              statement->type_id());
                    return false;
                }

                spillop->get_operand()->get_live_range().set_end(i);

                break;
            }
        }
    }

    return true;
}

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

MATHSEXPR_NAMESPACE_END