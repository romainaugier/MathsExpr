#include "mathsexpr/parser.h"

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
            ParserToken token = { (char*)&expression[i], 1, ParserTokenType_Variable };
            vector_push_back(tokens, &token);

            i++;
        }
        else if(is_operator(expression[i]))
        {
            ParserToken token = { (char*)&expression[i], 1, ParserTokenType_Operator };
            vector_push_back(tokens, &token);

            i++;
        }
        else if(is_paren(expression[i]))
        {
            ParserToken token = { (char*)&expression[i], 1, expression[i] == '(' ? ParserTokenType_LParen : ParserTokenType_RParen };
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

void mathsexpr_debug_tokens(Vector* tokens)
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
            case ParserTokenType_Operator:
                printf("OPERATOR: %.*s\n", token->size, token->start);
                break;
            case ParserTokenType_Variable:
                printf("VARIABLE: %.*s\n", token->size, token->start);
                break;

            default:
                break;
        }
    }
}

uint32_t mathsexpr_parse(const char* expression,
                         uint32_t expression_size)
{
    Vector* tokens = vector_new(128, sizeof(ParserToken));

    uint32_t ret = mathsexpr_lex(expression, expression_size, tokens);

    if(ret != 0)
    {
        return ret;
    }

    printf("%s\n", expression);
    mathsexpr_debug_tokens(tokens);

    return 0;
}