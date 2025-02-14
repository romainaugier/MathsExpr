#include "mathsexpr/parser.h"

#include <string.h>

int main(void)
{
    const char* expr1 = "a * 3 + 57 * (b + 34)";
    const size_t expr1_size = strlen(expr1);

    if(mathsexpr_parse(expr1, expr1_size) != 0)
    {
        return 1;
    }

    const char* expr2 = "2x - 7";
    const size_t expr2_size = strlen(expr2);

    if(mathsexpr_parse(expr2, expr2_size) != 0)
    {
        return 1;
    }

    const char* expr3 = "4x^2 + 7x + 2";
    const size_t expr3_size = strlen(expr3);

    if(mathsexpr_parse(expr3, expr3_size) != 0)
    {
        return 1;
    }

    return 0;
}