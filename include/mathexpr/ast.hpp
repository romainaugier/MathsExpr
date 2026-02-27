// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_AST)
#define __MATHEXPR_AST

#include "mathexpr/lexer.hpp"

#include <memory>
#include <iostream>
#include <iterator>
#include <optional>

MATHEXPR_NAMESPACE_BEGIN

enum ASTNodeTypeId : int
{
    ASTNodeTypeId_Variable = 1,
    ASTNodeTypeId_Literal = 2,
    ASTNodeTypeId_UnOp = 3,
    ASTNodeTypeId_BinOp = 4,
    ASTNodeTypeId_FuncOp = 5,
};

class MATHEXPR_API ASTNode
{
    /* Needed for Sethi-Ullman */
    bool _needs_reg;

public:
    ASTNode(const bool needs_reg = false) : _needs_reg(needs_reg) {}

    virtual ~ASTNode() = default;

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept = 0;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept = 0;

    virtual int type_id() const noexcept = 0;

    bool get_needs_reg() const noexcept { return this->_needs_reg; }

    void set_needs_reg(const bool needs_reg) noexcept { this->_needs_reg = needs_reg; }
};

class MATHEXPR_API ASTNodeVariable : public ASTNode
{
    std::string_view _name;

public:
    ASTNodeVariable(std::string_view name) : _name(name) {}

    virtual ~ASTNodeVariable() override {}

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept override;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept override 
    { 
        return std::nullopt;
    }

    static constexpr int static_type_id() { return ASTNodeTypeId_Variable; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    std::string_view get_name() const noexcept { return this->_name; }
};

class MATHEXPR_API ASTNodeLiteral : public ASTNode
{
    std::string_view _name;

    double _value;

public:
    ASTNodeLiteral(double value, std::string_view name) : _name(name), _value(value) {}

    virtual ~ASTNodeLiteral() override {}

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept override;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept override 
    { 
        return std::nullopt;
    }

    static constexpr int static_type_id() { return ASTNodeTypeId_Literal; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    double get_value() const noexcept { return this->_value; }

    std::string_view get_name() const noexcept { return this->_name; }
};

class MATHEXPR_API ASTNodeUnaryOp : public ASTNode
{
    std::shared_ptr<ASTNode> _operand;

    uint32_t _op;

public:
    ASTNodeUnaryOp(std::shared_ptr<ASTNode> operand, 
                   uint32_t op) : ASTNode(true),
                                  _operand(std::move(operand)),
                                  _op(op) {}

    virtual ~ASTNodeUnaryOp() override {}

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept override;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept override 
    {
        return std::vector<ASTNode*>({ this->_operand.get() }); 
    }

    static constexpr int static_type_id() { return ASTNodeTypeId_UnOp; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    const ASTNode* get_operand() const noexcept { return this->_operand.get(); }

    uint32_t get_op() const noexcept { return this->_op; }
};

class MATHEXPR_API ASTNodeBinaryOp : public ASTNode
{
    std::shared_ptr<ASTNode> _left;
    std::shared_ptr<ASTNode> _right;

    uint32_t _op;

public:
    ASTNodeBinaryOp(std::shared_ptr<ASTNode> left, 
                    std::shared_ptr<ASTNode> right, 
                    uint32_t op) : ASTNode(true),
                                   _left(std::move(left)),
                                   _right(std::move(right)),
                                   _op(op) {} 

    virtual ~ASTNodeBinaryOp() override {}

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept override;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept override 
    {
        return std::vector<ASTNode*>({ this->_left.get(), this->_right.get() }); 
    }

    static constexpr int static_type_id() { return ASTNodeTypeId_BinOp; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    const ASTNode* get_left() const noexcept { return this->_left.get(); }

    const ASTNode* get_right() const noexcept { return this->_right.get(); }

    uint32_t get_op() const noexcept { return this->_op; }
};

class MATHEXPR_API ASTNodeFunctionOp : public ASTNode
{
    std::vector<std::shared_ptr<ASTNode>> _arguments;

    std::string_view _name;

public:
    ASTNodeFunctionOp(std::string_view name, 
                        std::vector<std::shared_ptr<ASTNode>> arguments) : ASTNode(true),
                                                                           _arguments(std::move(arguments)), 
                                                                           _name(name) {}

    virtual ~ASTNodeFunctionOp() override {}

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept override;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept override 
    { 
        std::vector<ASTNode*> children(this->_arguments.size());

        for(const auto& argument : this->_arguments)
        {
            children.push_back(argument.get());
        }

        return children;
    }

    static constexpr int static_type_id() { return ASTNodeTypeId_FuncOp; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    std::string_view get_function_name() const noexcept { return this->_name; }

    const std::vector<std::shared_ptr<ASTNode>>& get_arguments() const noexcept { return this->_arguments; }

    size_t get_arguments_count() const noexcept { return this->_arguments.size(); }
};

template<typename T>
const T* node_cast(const ASTNode* node) noexcept
{
    if(node != nullptr && node->type_id() == T::static_type_id())
    {
        return static_cast<const T*>(node);
    }

    return nullptr;
}

class MATHEXPR_API AST
{
public:
    static constexpr size_t PRINT_INDENT_SIZE = 4;

private:
    std::shared_ptr<ASTNode> _root;

public:
    AST() {}

    ASTNode* get_root() noexcept { return this->_root.get(); }

    const ASTNode* get_root() const noexcept { return this->_root.get(); }

    void print() const noexcept;

    bool build_from_tokens(const LexerTokens& tokens) noexcept;

    void clear() noexcept 
    {
        if(this->_root)
        {
            this->_root.reset();
        }
    }
};

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_AST) */