// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/ast.h"

#include "libromano/stack_no_alloc.h"

#include <string.h>

AST* mathsexpr_ast_new()
{
    AST* new_ast = (AST*)malloc(sizeof(AST));
    mathsexpr_arena_init(&new_ast->nodes, 4096);

    return new_ast;
}

ASTNode* mathsexpr_ast_new_literal(AST* ast, float value)
{
    ASTLiteral lit = { ASTNodeType_ASTLiteral, value };

    return (ASTNode*)mathsexpr_arena_push(&ast->nodes, &lit, sizeof(ASTLiteral));
}

ASTNode* mathsexpr_ast_new_variable(AST* ast, char name)
{
    ASTVariable var = { ASTNodeType_ASTVariable, name };

    return (ASTNode*)mathsexpr_arena_push(&ast->nodes, &var, sizeof(ASTVariable));
}

ASTNode* mathsexpr_ast_new_binop(AST* ast, ASTBinOPType op, ASTNode* left, ASTNode* right)
{
    ASTBinOP binop = { ASTNodeType_ASTBinOP, op, left, right };

    return (ASTNode*)mathsexpr_arena_push(&ast->nodes, &binop, sizeof(ASTBinOP));
}

ASTNode* mathsexpr_ast_new_unop(AST* ast, ASTUnOPType op, ASTNode* operand)
{
    ASTUnOP unop = { ASTNodeType_ASTUnOP, op, operand };

    return (ASTNode*)mathsexpr_arena_push(&ast->nodes, &unop, sizeof(ASTUnOP));
}

ASTBinOPType mathsexpr_ast_token_op_to_binop(const ParserToken* token)
{
    char op_char = *(token->start);

    switch (op_char) 
    {
        case '+':  
            return ASTBinOPType_Add;
        case '-':  
            return ASTBinOPType_Sub;
        case '*':  
            return ASTBinOPType_Mul;
        case '/':  
            return ASTBinOPType_Div;
        case '%':  
            return ASTBinOPType_Mod;
        case '^':  
            return ASTBinOPType_Pow;
        default:   
            return ASTBinOPType_Add;
    }
}

uint32_t mathsexpr_ast_get_binop_precedence(ASTBinOPType op)
{
    switch(op) 
    {
        case ASTBinOPType_Pow:
            return 4;
        case ASTBinOPType_Mul:
        case ASTBinOPType_Div:
        case ASTBinOPType_Mod:         
            return 3;
        case ASTBinOPType_Add:
        case ASTBinOPType_Sub:         
            return 2;
        default:
            return 0;
    }
}

#define SHUNTING_YARD_STACK_SIZE 256

bool mathsexpr_ast_from_infix_parser_tokens(AST* ast, Vector* tokens)
{
    stack_init(ASTNode*, nodes_stack, SHUNTING_YARD_STACK_SIZE);
    stack_init(ParserToken, operators_stack, SHUNTING_YARD_STACK_SIZE);
    size_t i = 0;

    while(i < vector_size(tokens))
    {
        ParserToken* token = vector_at(tokens, i);

        switch(token->token_type)
        {
            case ParserTokenType_Literal:
            {
                ASTNode* node = mathsexpr_ast_new_literal(ast, strtof(token->start, NULL));
                stack_push(nodes_stack, node);
                break;
            }

            case ParserTokenType_Variable:
            {
                ASTNode* node = mathsexpr_ast_new_variable(ast, *(token->start));
                stack_push(nodes_stack, node);
                break;
            }

            case ParserTokenType_BinOperator:
            {
                uint32_t precedence = mathsexpr_parser_get_operator_precedence(*(token->start));

                while(!stack_is_empty(operators_stack))
                {
                    uint32_t previous_precedence = mathsexpr_parser_get_operator_precedence(*(stack_top(operators_stack)->start));

                    if(previous_precedence >= precedence)
                    {
                        ParserToken tmp = stack_pop(operators_stack);

                        ASTNode* right = stack_pop(nodes_stack);
                        ASTNode* left = stack_pop(nodes_stack);

                        ASTNode* binop = mathsexpr_ast_new_binop(ast, mathsexpr_ast_token_op_to_binop(&tmp), left, right);
                        stack_push(nodes_stack, binop);
                    }
                    else
                    {
                        break;
                    }
                }

                stack_push(operators_stack, *token);

                break;
            }

            case ParserTokenType_LParen:
                stack_push(operators_stack, *token);
                break;

            case ParserTokenType_RParen:
            {
                while(stack_top(operators_stack) != NULL && 
                      stack_top(operators_stack)->token_type != ParserTokenType_LParen)
                {
                    ParserToken tmp = stack_pop(operators_stack);

                    ASTNode* right = stack_pop(nodes_stack);
                    ASTNode* left = stack_pop(nodes_stack);

                    ASTNode* binop = mathsexpr_ast_new_binop(ast, mathsexpr_ast_token_op_to_binop(&tmp), left, right);
                    stack_push(nodes_stack, binop);
                }

                if(stack_top(operators_stack) == NULL)
                {
                    return false;
                }

                stack_pop(operators_stack);

                break;
            }
        }

        i++;
    }

    while(!stack_is_empty(operators_stack))
    {
        ParserToken tmp = stack_pop(operators_stack);

        ASTNode* right = stack_pop(nodes_stack);
        ASTNode* left = stack_pop(nodes_stack);

        ASTNode* binop = mathsexpr_ast_new_binop(ast, mathsexpr_ast_token_op_to_binop(&tmp), left, right);
        stack_push(nodes_stack, binop);
    }

    ast->root = stack_pop(nodes_stack);

    return true;
}

bool mathsexpr_ast_from_postfix_parser_tokens(AST* ast, Vector* tokens)
{
    size_t i = 0;

    while(i < vector_size(tokens))
    {

    }

    return true;
}

const char* binop_to_str(ASTBinOPType op) 
{
    switch(op) 
    {
        case ASTBinOPType_Add: 
            return "+";
        case ASTBinOPType_Sub: 
            return "-";
        case ASTBinOPType_Mul: 
            return "*";
        case ASTBinOPType_Div: 
            return "/";
        case ASTBinOPType_Mod: 
            return "%%";
        case ASTBinOPType_Pow: 
            return "^";
        default: 
            return "?";
    }
}

void ast_print_recursive(ASTNode* node, int depth) 
{
    if(node == NULL)
    {
        return;
    }

    for (int i = 0; i < depth; i++) 
    {
        printf("  ");
    }

    switch (node->type) 
    {
        case ASTNodeType_ASTLiteral:
            printf("AST LITERAL: %.2f\n", ((ASTLiteral*)node)->value);
            break;
            
        case ASTNodeType_ASTVariable:
            printf("AST VARIABLE: %c\n", ((ASTVariable*)node)->name);
            break;
            
        case ASTNodeType_ASTBinOP:
        {
            ASTBinOP* binop = (ASTBinOP*)node;
            printf("AST BINOP: %s\n", binop_to_str(binop->op));
            ast_print_recursive(binop->left, depth + 1);
            ast_print_recursive(binop->right, depth + 1);
            break;
        }
            
        case ASTNodeType_ASTUnOP: 
        {
            ASTUnOP* unop = (ASTUnOP*)node;
            printf("AST UNOP\n");
            ast_print_recursive(unop->operand, depth + 1);
            break;
        }
    }
}

void mathsexpr_ast_print(AST* ast) 
{
    ast_print_recursive(ast->root, 0);
}

void mathsexpr_ast_destroy(AST* ast)
{
    mathsexpr_arena_destroy(&ast->nodes);
    free(ast);
}