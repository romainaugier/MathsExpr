// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_AST)
#define __MATHSEXPR_AST

#include "mathsexpr/lexer.h"

#include <memory>
#include <iostream>
#include <iterator>
#include <optional>

MATHSEXPR_NAMESPACE_BEGIN

class MATHSEXPR_API ASTNode
{
public:
    virtual ~ASTNode() = default;

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept = 0;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept = 0;

    virtual int type_id() const noexcept = 0;
};

class MATHSEXPR_API ASTNodeVariable : public ASTNode
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

    static constexpr int static_type_id() { return 1; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    std::string_view get_name() const noexcept { return this->_name; }
};

class MATHSEXPR_API ASTNodeLiteral : public ASTNode
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

    static constexpr int static_type_id() { return 2; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    double get_value() const noexcept { return this->_value; }

    std::string_view get_name() const noexcept { return this->_name; }
};

class MATHSEXPR_API ASTNodeFunctionCall : public ASTNode
{
    std::vector<std::shared_ptr<ASTNode>> _arguments;

    std::string_view _name;

public:
    ASTNodeFunctionCall(std::string_view name, 
                        std::vector<std::shared_ptr<ASTNode>> arguments) : _arguments(std::move(arguments)), 
                                                                           _name(name) {}

    virtual ~ASTNodeFunctionCall() override {}

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

    static constexpr int static_type_id() { return 3; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    std::string_view get_function_name() const noexcept { return this->_name; }

    size_t get_arguments_count() const noexcept { return this->_arguments.size(); }
};

enum UnaryOpType : uint32_t
{
    UnaryOpType_Unknown,
    UnaryOpType_Neg,
};

class MATHSEXPR_API ASTNodeUnaryOp : public ASTNode
{
    std::shared_ptr<ASTNode> _operand;

    uint32_t _op;

public:
    ASTNodeUnaryOp(std::shared_ptr<ASTNode> operand, uint32_t op) : _operand(std::move(operand)), _op(op) {}

    virtual ~ASTNodeUnaryOp() override {}

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept override;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept override 
    {
        return std::vector<ASTNode*>({ this->_operand.get() }); 
    }

    static constexpr int static_type_id() { return 4; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }
};

enum BinaryOpType : uint32_t 
{
    BinaryOpType_Unknown,
    BinaryOpType_Add,
    BinaryOpType_Sub,
    BinaryOpType_Mul,
    BinaryOpType_Div,
};

class MATHSEXPR_API ASTNodeBinaryOp : public ASTNode
{
    std::shared_ptr<ASTNode> _left;
    std::shared_ptr<ASTNode> _right;

    uint32_t _op;

public:
    ASTNodeBinaryOp(std::shared_ptr<ASTNode> left, 
                    std::shared_ptr<ASTNode> right, 
                    uint32_t op) : _left(std::move(left)), _right(std::move(right)), _op(op) {} 

    virtual ~ASTNodeBinaryOp() override {}

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept override;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept override 
    {
        return std::vector<ASTNode*>({ this->_left.get(), this->_right.get() }); 
    }

    static constexpr int static_type_id() { return 5; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }
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

class MATHSEXPR_API AST
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

    template<typename N, typename... Args>
    std::shared_ptr<N> new_node(Args&&... args) const noexcept
    {
        return std::make_shared<N>(std::forward<Args>(args)...);
    }
};

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_AST) */