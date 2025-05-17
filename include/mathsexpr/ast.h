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

    double get_value() const noexcept { return this->_value; }

    std::string_view get_name() const noexcept { return this->_name; }
};

class MATHSEXPR_API ASTNodeFunctionCall : public ASTNode
{
    std::vector<ASTNode*> _arguments;

    std::string_view _name;

public:
    ASTNodeFunctionCall(std::vector<ASTNode*>& arguments, 
                        std::string_view name) : _arguments(std::move(arguments)), 
                                                 _name(name) {}

    virtual ~ASTNodeFunctionCall() override {}

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept override;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept override 
    { 
        return this->_arguments;
    }

    std::string_view get_function_name() const noexcept { return this->_name; }

    size_t get_arguments_count() const noexcept { return this->_arguments.size(); }
};

enum UnaryOpType : uint32_t
{
    Neg,
};

class MATHSEXPR_API ASTNodeUnaryOp : public ASTNode
{
    ASTNode* _operand;

    uint32_t _op;

public:
    ASTNodeUnaryOp(ASTNode* operand, uint32_t op) : _operand(operand), _op(op) {}

    virtual ~ASTNodeUnaryOp() override {}

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept override;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept override 
    {
        return std::vector<ASTNode*>({ this->_operand }); 
    }
};

enum BinaryOpType : uint32_t 
{
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Pow,
};

class MATHSEXPR_API ASTNodeBinaryOp : public ASTNode
{
    ASTNode* _left;
    ASTNode* _right;

    uint32_t _op;

public:
    ASTNodeBinaryOp(ASTNode* left, ASTNode* right, uint32_t op) : _left(left), _right(right), _op(op) {} 

    virtual ~ASTNodeBinaryOp() override {}

    virtual void print(std::ostream_iterator<char>& out, size_t indent) const noexcept override;

    virtual std::optional<std::vector<ASTNode*>> get_children() const noexcept override 
    {
        return std::vector<ASTNode*>({ this->_left, this->_right }); 
    }
};

class MATHSEXPR_API AST
{
public:
    static constexpr size_t PRINT_INDENT_SIZE = 4;

private:
    std::unique_ptr<ASTNode> _root;

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
            this->_root.release();
        }
    }

    template<typename N, typename... Args>
    std::unique_ptr<N> new_node(Args&&... args) const noexcept
    {
        return std::make_unique<N>(std::forward<Args>(args)...);
    }
};

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_AST) */