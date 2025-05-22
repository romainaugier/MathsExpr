// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/ssa.h"
#include "mathsexpr/op.h"

#include <ranges>
#include <unordered_map>

MATHSEXPR_NAMESPACE_BEGIN

void SSAStmtVariable::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out, "{}{} = {}\n", VERSION_CHAR, this->get_version(), this->_name);
}

void SSAStmtLiteral::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out, "{}{} = {}\n", VERSION_CHAR, this->get_version(), this->_name);
}

void SSAStmtUnOp::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out, 
                   "{}{} = {}{}{}\n",
                   VERSION_CHAR,
                   this->get_version(),
                   op_unary_to_string(this->_op),
                   VERSION_CHAR,
                   this->_operand->get_version());
}

void SSAStmtBinOp::print(std::ostream_iterator<char>& out) const noexcept
{
    std::format_to(out, 
                   "{}{} = {}{} {} {}{}\n",
                   VERSION_CHAR,
                   this->get_version(),
                   VERSION_CHAR,
                   this->_left->get_version(),
                   op_binary_to_string(this->_op),
                   VERSION_CHAR,
                   this->_right->get_version());
}

void SSAStmtFunctionCall::print(std::ostream_iterator<char>& out) const noexcept
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

    std::format_to(out, "{}{} = {}({})\n", VERSION_CHAR, this->get_version(), this->_name, arguments);
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

uint64_t SSAStmtFunctionCall::canonicalize() const noexcept
{
    return 0;
}

/* SSA */

bool SSA::build_from_ast(const AST& ast) noexcept
{
    this->_statements.clear();
    uint64_t version = 1;
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
                                                                        version++);

                this->_statements.push_back(variable);
                mapping[variable_node] = variable;

                break;
            }
            case ASTNodeTypeId_Literal:
            {
                const ASTNodeLiteral* literal_node = node_cast<ASTNodeLiteral>(node);

                SSAStmtPtr literal = std::make_shared<SSAStmtLiteral>(literal_node->get_name(),
                                                                      version++);

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

                SSAStmtPtr unop = std::make_shared<SSAStmtUnOp>(mapping[unop_node->get_operand()],
                                                                unop_node->get_op(),
                                                                version++);

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

                SSAStmtPtr binop = std::make_shared<SSAStmtBinOp>(mapping[binop_node->get_left()],
                                                                  mapping[binop_node->get_right()],
                                                                  binop_node->get_op(),
                                                                  version++);

                this->_statements.push_back(binop);
                mapping[binop_node] = binop;

                break;
            }
            case ASTNodeTypeId_FunctionCall:
            {
                const ASTNodeFunctionCall* funccall_node = node_cast<ASTNodeFunctionCall>(node);

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

                SSAStmtPtr funccall = std::make_shared<SSAStmtFunctionCall>(funccall_node->get_function_name(),
                                                                            std::move(arguments),
                                                                            version++);

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