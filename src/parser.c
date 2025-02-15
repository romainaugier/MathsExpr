// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/parser.h"

#include "libromano/stack_no_alloc.h"

#include <string.h>

MATHSEXPR_FORCE_INLINE bool is_digit(unsigned int c)
{
    return (c - 48) < 10;
}

MATHSEXPR_FORCE_INLINE bool is_dot(unsigned int c)
{
    return c == '.';
}

MATHSEXPR_FORCE_INLINE bool is_letter(unsigned int c)
{
    return ((c & ~0x20) - 65) < 26;
}

MATHSEXPR_FORCE_INLINE bool is_operator(unsigned int c)
{
    return c == '+' |
           c == '-' |
           c == '*' |
           c == '/' |
           c == '%' | 
           c == '^';
}

MATHSEXPR_FORCE_INLINE bool is_paren(unsigned int c)
{
    return c == '(' | c == ')';
}

uint32_t mathsexpr_lex(const char* expression,
                       uint32_t expression_size,
                       Vector* tokens)
{
    uint32_t i = 0;

    while(i < expression_size)
    {
        if(is_digit(expression[i]))
        {
            char* start = (char*)&expression[i];
            uint32_t size = 1;
            i++;

            bool already_has_dot = false;

            while(i < expression_size && (is_digit(expression[i]) || is_dot(expression[i])))
            {
                if(is_dot(expression[i]))
                {
                    if(already_has_dot)
                    {
                        return 1;
                    }

                    already_has_dot = true;
                }

                i++;
                size++;
            }

            ParserToken token = { start, size, ParserTokenType_Literal };
            vector_push_back(tokens, &token);
        }
        else if(is_letter(expression[i]))
        {
            if(vector_size(tokens) > 0 && 
               ((ParserToken*)vector_back(tokens))->token_type == ParserTokenType_Literal)
            {
                ParserToken mul_token = { (char*)"*", 1, ParserTokenType_BinOperator };
                vector_push_back(tokens, &mul_token);
            }

            ParserToken token = { (char*)&expression[i], 1, ParserTokenType_Variable };
            vector_push_back(tokens, &token);

            i++;
        }
        else if(is_operator(expression[i]))
        {
            bool is_binary_operator = true;

            if(vector_size(tokens) == 0 ||
               (((ParserToken*)vector_back(tokens))->token_type != ParserTokenType_Literal &&
                ((ParserToken*)vector_back(tokens))->token_type != ParserTokenType_Variable && 
                ((ParserToken*)vector_back(tokens))->token_type != ParserTokenType_RParen))
            {
                is_binary_operator = false;
            }

            ParserToken token = { (char*)&expression[i], 1, is_binary_operator ? ParserTokenType_BinOperator : 
                                                                                 ParserTokenType_UnOperator };
            vector_push_back(tokens, &token);

            i++;
        }
        else if(is_paren(expression[i]))
        {
            ParserToken token = { (char*)&expression[i], 1, expression[i] == '(' ? ParserTokenType_LParen : 
                                                                                   ParserTokenType_RParen };
            vector_push_back(tokens, &token);

            i++;
        }
        else
        {
            i++;
        }
    }

    return 0;
}

uint32_t mathsexpr_parser_parse(const char* expression,
                                uint32_t expression_size,
                                Vector* tokens)
{
    uint32_t ret = mathsexpr_lex(expression, expression_size, tokens);

    if(ret != 0)
    {
        return ret;
    }

    uint32_t i = 0;

    while(i < vector_size(tokens))
    {
        if(((ParserToken*)vector_at(tokens, i))->token_type != ParserTokenType_UnOperator)
        {
            i++;
            continue;
        }

        ParserToken unary_token = *((ParserToken*)vector_at(tokens, i));

        ParserToken lparen_token = { "(", 1, ParserTokenType_LParen };
        memcpy(vector_at(tokens, i), &lparen_token, sizeof(ParserToken));

        ParserToken zero_token = { "0", 1, ParserTokenType_Literal };
        vector_insert(tokens, &zero_token, i + 1);

        ParserToken op_token = { unary_token.start, 1, ParserTokenType_BinOperator };
        vector_insert(tokens, &op_token, i + 2);

        ParserToken rparen_token = { ")", 1, ParserTokenType_RParen };
        vector_insert(tokens, &rparen_token, i + 4);

        i += 5;
    }

    return 0;
}

uint32_t mathsexpr_parser_get_operator_precedence(char operator)
{
    switch(operator) 
    {
        case '^':
            return 4;
        case '*':
        case '/':
        case '%':         
            return 3;
        case '+':
        case '-':         
            return 2;
        default:
            return 0;
    }
}

bool mathsexpr_is_operator_right_associative(char operator)
{
    return operator == '^';
}

#define SHUNTING_YARD_STACK_SIZE 256

bool mathsexpr_parser_infix_to_postfix(Vector* infix_tokens, Vector* postfix_tokens)
{
    stack_init(ParserToken, operators_stack, SHUNTING_YARD_STACK_SIZE);

    size_t i = 0;

    while(i < vector_size(infix_tokens))
    {
        ParserToken* token = vector_at(infix_tokens, i);

        switch(token->token_type)
        {
            case ParserTokenType_Literal:
            case ParserTokenType_Variable:
                vector_push_back(postfix_tokens, token);
                break;

            case ParserTokenType_BinOperator:
            {
                uint32_t precedence = mathsexpr_parser_get_operator_precedence(*token->start);

                while(!stack_is_empty(operators_stack))
                {
                    uint32_t previous_precedence = mathsexpr_parser_get_operator_precedence(*(stack_top(operators_stack)->start));

                    if(previous_precedence >= precedence)
                    {
                        ParserToken tmp = stack_pop(operators_stack);
                        vector_push_back(postfix_tokens, &tmp);
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
                    vector_push_back(postfix_tokens, &tmp);
                }

                if(stack_top(operators_stack) == NULL)
                {
                    return false;
                }

                stack_pop(operators_stack);

                break;
            }
            
            default:
                break;
        }

        i++;
    }

    while(!stack_is_empty(operators_stack))
    {
        ParserToken tmp = stack_pop(operators_stack);
        vector_push_back(postfix_tokens, &tmp);
    }

    return true;
}

void mathsexpr_parser_debug_tokens(Vector* tokens)
{
    for(uint32_t i = 0; i < vector_size(tokens); i++)
    {
        ParserToken* token = (ParserToken*)vector_at(tokens, i);

        switch(token->token_type)
        {
            case ParserTokenType_Literal:
                printf("LITERAL: %.*s\n", token->size, token->start);
                break;
            case ParserTokenType_LParen:
                printf("LPAREN: %.*s\n", token->size, token->start);
                break;
            case ParserTokenType_RParen:
                printf("RPAREN: %.*s\n", token->size, token->start);
                break;
            case ParserTokenType_BinOperator:
                printf("BINARY OPERATOR: %.*s\n", token->size, token->start);
                break;
            case ParserTokenType_UnOperator:
                printf("UNARY OPERATOR: %.*s\n", token->size, token->start);
                break;
            case ParserTokenType_Variable:
                printf("VARIABLE: %.*s\n", token->size, token->start);
                break;

            default:
                break;
        }
    }
}
