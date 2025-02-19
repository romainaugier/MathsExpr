// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/ssa.h"

#include "libromano/logger.h"
#include "libromano/string.h"
#include "libromano/flag.h"

#include <string.h>

void ssa_func_lookup_table_init(HashMap* func_lookup_table)
{
    uint32_t value = (uint32_t)SSAFuncType_Tanh;
    hashmap_insert(func_lookup_table, "tanh", 4, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Sinh;
    hashmap_insert(func_lookup_table, "sinh", 4, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Cosh;
    hashmap_insert(func_lookup_table, "cosh", 4, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Tan;
    hashmap_insert(func_lookup_table, "tan", 3, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Sin;
    hashmap_insert(func_lookup_table, "sin", 3, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Cos;
    hashmap_insert(func_lookup_table, "cos", 3, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Atan;
    hashmap_insert(func_lookup_table, "atan", 4, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Asin;
    hashmap_insert(func_lookup_table, "asin", 4, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Acos;
    hashmap_insert(func_lookup_table, "acos", 4, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Frac;
    hashmap_insert(func_lookup_table, "frac", 4, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Ceil;
    hashmap_insert(func_lookup_table, "ceil", 4, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Floor;
    hashmap_insert(func_lookup_table, "floor", 5, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Log10;
    hashmap_insert(func_lookup_table, "log10", 5, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Log2;
    hashmap_insert(func_lookup_table, "log2", 4, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Log;
    hashmap_insert(func_lookup_table, "log", 3, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Rsqrt;
    hashmap_insert(func_lookup_table, "rsqrt", 5, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Rcp;
    hashmap_insert(func_lookup_table, "rcp", 3, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Sqrt;
    hashmap_insert(func_lookup_table, "sqrt", 4, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Exp;
    hashmap_insert(func_lookup_table, "exp", 3, &value, sizeof(uint32_t));
    value = (uint32_t)SSAFuncType_Abs;
    hashmap_insert(func_lookup_table, "abs", 3, &value, sizeof(uint32_t));
}

uint32_t* ssa_get_func_type(SSA* ssa, const char* func_name, uint32_t func_name_length)
{
    return hashmap_get(ssa->functions_lookup_table, func_name, func_name_length, NULL);
}

SSA* mathsexpr_ssa_new()
{
    SSA* new_ssa = (SSA*)malloc(sizeof(SSA));
    mathsexpr_arena_init(&new_ssa->instructions_data, 4096);

    new_ssa->instructions = vector_new(128, sizeof(SSAInstruction*));
    new_ssa->counter = 0;
    new_ssa->functions_lookup_table = hashmap_new(128);
    new_ssa->flags = 0;

    ssa_func_lookup_table_init(new_ssa->functions_lookup_table);

    return new_ssa;
}

SSAInstruction* mathsexpr_ssa_new_literal(SSA* ssa, 
                                          float value, 
                                          uint32_t destination)
{
    SSALiteral lit = { SSAInstructionType_SSALiteral, value, destination };

    SSAInstruction* instruction_ptr = (SSAInstruction*)mathsexpr_arena_push(&ssa->instructions_data, &lit, sizeof(SSALiteral));

    vector_push_back(ssa->instructions, &instruction_ptr);

    return instruction_ptr;
}

SSAInstruction* mathsexpr_ssa_new_variable(SSA* ssa,
                                           char* name,
                                           uint32_t name_length,
                                           uint32_t destination)
{
    SSAVariable var;
    var.base.type = SSAInstructionType_SSAVariable; 
    memcpy(&var.name, name, name_length);
    var.name_length = name_length;
    var.destination = destination;

    SSAInstruction* instruction_ptr = (SSAInstruction*)mathsexpr_arena_push(&ssa->instructions_data, &var, sizeof(SSAVariable));

    vector_push_back(ssa->instructions, &instruction_ptr);

    return instruction_ptr;
}

SSAInstruction* mathsexpr_ssa_new_binop(SSA* ssa, 
                                        SSABinOPType op, 
                                        SSAInstruction* left,
                                        SSAInstruction* right,
                                        uint32_t destination)
{
    SSABinOP binop = { SSAInstructionType_SSABinOP, op, left, right, destination };

    SSAInstruction* instruction_ptr = (SSAInstruction*)mathsexpr_arena_push(&ssa->instructions_data, &binop, sizeof(SSABinOP));

    vector_push_back(ssa->instructions, &instruction_ptr);

    return instruction_ptr;
}

SSAInstruction* mathsexpr_ssa_new_unop(SSA* ssa,
                                       SSAUnOPType op,
                                       SSAInstruction* operand,
                                       uint32_t destination)
{
    SSAUnOP unop = { SSAInstructionType_SSAUnOP, op, operand, destination };

    SSAInstruction* instruction_ptr = (SSAInstruction*)mathsexpr_arena_push(&ssa->instructions_data, &unop, sizeof(SSAUnOP));

    vector_push_back(ssa->instructions, &instruction_ptr);

    return instruction_ptr;
}

SSAInstruction* mathsexpr_ssa_new_function(SSA* ssa,
                                           SSAFuncType type,
                                           SSAInstruction* argument,
                                           uint32_t destination)
{
    SSAFunction func = { SSAInstructionType_SSAFunction, type, argument, destination };

    SSAInstruction* instruction_ptr = (SSAInstruction*)mathsexpr_arena_push(&ssa->instructions_data, &func, sizeof(SSAFunction));

    vector_push_back(ssa->instructions, &instruction_ptr);

    return instruction_ptr;
}

SSABinOPType ast_binop_to_ssa_binop(ASTBinOPType type)
{
    switch(type)
    {
        case ASTBinOPType_Add:
            return SSABinOPType_Add;
        case ASTBinOPType_Sub:
            return SSABinOPType_Sub;
        case ASTBinOPType_Mul:
            return SSABinOPType_Mul;
        case ASTBinOPType_Div:
            return SSABinOPType_Div;
        case ASTBinOPType_Pow:
            return SSABinOPType_Pow;
        case ASTBinOPType_Mod:
            return SSABinOPType_Mod;
        default:
            return 0;
    }
}

SSAUnOPType ast_unop_to_ssa_unop(ASTUnOPType type)
{
    switch(type)
    {
        case ASTUnOPType_Neg:
            return SSAUnOPType_Neg;
        default:
            return 0;
    }
}

SSAInstruction* ssa_from_ast_node(SSA* ssa, ASTNode* node)
{
    uint32_t destination = ssa->counter++;

    switch(node->type)
    {
        case ASTNodeType_ASTLiteral:
        {
            ASTLiteral* lit = AST_CAST(ASTLiteral, node);
            MATHSEXPR_ASSERT(lit != NULL, "Wrong type casting, should be ASTLiteral");

            return mathsexpr_ssa_new_literal(ssa, lit->value, destination);
        }
        case ASTNodeType_ASTVariable:
        {
            ASTVariable* var = AST_CAST(ASTVariable, node);
            MATHSEXPR_ASSERT(var != NULL, "Wrong type casting, should be ASTVariable");

            return mathsexpr_ssa_new_variable(ssa, var->name, var->name_length, destination);
        }
        case ASTNodeType_ASTBinOP:
        {
            ASTBinOP* binop = AST_CAST(ASTBinOP, node);
            MATHSEXPR_ASSERT(binop != NULL, "Wrong type casting, should be ASTBinOP");

            return mathsexpr_ssa_new_binop(ssa,
                                           ast_binop_to_ssa_binop(binop->op),
                                           ssa_from_ast_node(ssa, binop->left),
                                           ssa_from_ast_node(ssa, binop->right),
                                           destination);
        }
        case ASTNodeType_ASTUnOP:
        {
            ASTUnOP* unop = AST_CAST(ASTUnOP, node);
            MATHSEXPR_ASSERT(unop != NULL, "Wrong type casting, should be ASTUnOP");

            return mathsexpr_ssa_new_unop(ssa,
                                          ast_binop_to_ssa_binop(unop->op),
                                          ssa_from_ast_node(ssa, unop->operand),
                                          destination);
        }
        case ASTNodeType_ASTFunction:
        {
            ASTFunction* func = AST_CAST(ASTFunction, node);
            MATHSEXPR_ASSERT(func != NULL, "Wrong type casting, should be ASTFunction");

            uint32_t* func_type = ssa_get_func_type(ssa, func->name, func->name_length);

            if(func_type == NULL)
            {
                logger_log_error("Unknown function: %.*s", func->name_length, func->name);
                return NULL;
            }

            return mathsexpr_ssa_new_function(ssa,
                                              *func_type,
                                              ssa_from_ast_node(ssa, func->argument),
                                              destination);
        }
        default:
            return NULL;
    }
}

bool mathsexpr_ssa_from_ast(SSA* ssa, AST* ast)
{
    ssa_from_ast_node(ssa, ast->root);

    return true;
}

const char* ssa_binop_type_as_string(SSABinOPType type)
{
    switch(type)
    {
        case SSABinOPType_Add:
            return "+";
        case SSABinOPType_Sub:
            return "-";
        case SSABinOPType_Mul:
            return "*";
        case SSABinOPType_Div:
            return "/";
        case SSABinOPType_Pow:
            return "^";
        case SSABinOPType_Mod:
            return "%%";
        default:
            return "?";
    }
}

const char* ssa_unop_type_as_string(SSAUnOPType type)
{
    switch(type)
    {
        case SSAUnOPType_Neg:
            return "-";
        default:
            return "?";
    }
}

const char* ssa_func_type_as_string(SSAFuncType type)
{
    switch(type)
    {
        case SSAFuncType_Abs:
            return "abs";
        case SSAFuncType_Exp:
            return "exp";
        case SSAFuncType_Sqrt:
            return "sqrt";
        case SSAFuncType_Rcp:
            return "rcp";
        case SSAFuncType_Rsqrt:
            return "rsqrt";
        case SSAFuncType_Log:
            return "log";
        case SSAFuncType_Log2:
            return "log2";
        case SSAFuncType_Log10:
            return "log10";
        case SSAFuncType_Floor:
            return "floor";
        case SSAFuncType_Ceil:
            return "ceil";
        case SSAFuncType_Frac:
            return "frac";
        case SSAFuncType_Acos:
            return "acos";
        case SSAFuncType_Asin:
            return "asin";
        case SSAFuncType_Atan:
            return "atan";
        case SSAFuncType_Cos:
            return "cos";
        case SSAFuncType_Sin:
            return "sin";
        case SSAFuncType_Tan:
            return "tan";
        case SSAFuncType_Cosh:
            return "cosh";
        case SSAFuncType_Sinh:
            return "sinh";
        case SSAFuncType_Tanh:
            return "tanh";
        default:
            return "unknown_func";
    }
}

uint32_t mathsexpr_ssa_get_instruction_destination(SSAInstruction* instruction)
{
    switch(instruction->type)
    {
        case SSAInstructionType_SSALiteral:
        {
            SSALiteral* lit = SSA_CAST(SSALiteral, instruction);
            return lit->destination;
        }
        case SSAInstructionType_SSAVariable:
        {
            SSAVariable* var = SSA_CAST(SSAVariable, instruction);
            return var->destination;
        }
        case SSAInstructionType_SSABinOP:
        {
            SSABinOP* binop = SSA_CAST(SSABinOP, instruction);
            return binop->destination;
        }
        case SSAInstructionType_SSAUnOP:
        {
            SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction);
            return unop->destination;
        }
        case SSAInstructionType_SSAFunction:
        {
            SSAFunction* func = SSA_CAST(SSAFunction, instruction);
            return func->destination;
        }
        default:
            return 0;
    }
}

void ssa_print_binop_inlined(SSABinOP* binop, char reg)
{
    String fmt_string = string_newf("%c%d = ", reg, binop->destination);

    if(binop->left->type == SSAInstructionType_SSALiteral)
    {
        SSALiteral* lit = SSA_CAST(SSALiteral, binop->left);
        string_appendf(&fmt_string, "%.3f", lit->value);
    }
    else
    {
        string_appendf(&fmt_string, "%c%u", reg, mathsexpr_ssa_get_instruction_destination(binop->left));
    }

    string_appendf(&fmt_string, " %s ", ssa_binop_type_as_string(binop->op));

    if(binop->right->type == SSAInstructionType_SSALiteral)
    {
        SSALiteral* lit = SSA_CAST(SSALiteral, binop->right);
        string_appendf(&fmt_string, "%.3f", lit->value);
    }
    else
    {
        string_appendf(&fmt_string, "%c%u", reg, mathsexpr_ssa_get_instruction_destination(binop->right));
    }

    puts(fmt_string); 

    string_free(fmt_string);
}

void ssa_print_function_inlined(SSAFunction* func, char reg)
{
    String fmt_string = string_newf("%c%d = call %s", reg, func->destination, ssa_func_type_as_string(func->func));

    if(func->argument->type == SSAInstructionType_SSALiteral)
    {
        SSALiteral* lit = SSA_CAST(SSALiteral, func->argument);
        string_appendf(&fmt_string, "(%.3f)", lit->value);
    }
    else
    {
        string_appendf(&fmt_string, "(%c%u)", reg, mathsexpr_ssa_get_instruction_destination(func->argument));
    }

    puts(fmt_string); 

    string_free(fmt_string);
}

void mathsexpr_ssa_print(SSA* ssa)
{
    char reg = HAS_FLAG(ssa->flags, SSAFlags_HasRegistersAsDestination) ? 'R' : 't';

    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

        switch(instruction->type)
        {
            case SSAInstructionType_SSALiteral:
            {
                SSALiteral* lit = SSA_CAST(SSALiteral, instruction);
                MATHSEXPR_ASSERT(lit != NULL, "Wrong type casting, should be SSALiteral");
                printf("%c%u = %.3f\n", reg, lit->destination, lit->value);
                break;
            }
            case SSAInstructionType_SSAVariable:
            {
                SSAVariable* var = SSA_CAST(SSAVariable, instruction);
                MATHSEXPR_ASSERT(var != NULL, "Wrong type casting, should be SSAVariable");
                printf("%c%u = %.*s\n", reg, var->destination, var->name_length, var->name);
                break;
            }
            case SSAInstructionType_SSABinOP:
            {
                SSABinOP* binop = SSA_CAST(SSABinOP, instruction);
                MATHSEXPR_ASSERT(binop != NULL, "Wrong type casting, should be SSABinOP");

                if(HAS_FLAG(ssa->flags, SSAFlags_InlineLiterals))
                {
                    ssa_print_binop_inlined(binop, reg);
                }
                else
                {
                    printf("%c%u = %c%u %s %c%u\n", 
                           reg,
                           binop->destination, 
                           reg,
                           mathsexpr_ssa_get_instruction_destination(binop->left),
                           ssa_binop_type_as_string(binop->op),
                           reg,
                           mathsexpr_ssa_get_instruction_destination(binop->right));
                }

                break;
            }
            case SSAInstructionType_SSAUnOP:
            {
                SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction);
                MATHSEXPR_ASSERT(unop != NULL, "Wrong type casting, should be SSAUnOP");

                printf("%c%u = %s%c%u\n", 
                       reg,
                       unop->destination, 
                       ssa_unop_type_as_string(unop->op), 
                       reg,
                       mathsexpr_ssa_get_instruction_destination(unop->operand));

                break;
            }
            case SSAInstructionType_SSAFunction:
            {
                SSAFunction* func = SSA_CAST(SSAFunction, instruction);
                MATHSEXPR_ASSERT(func != NULL, "Wrong type casting, should be SSAFunction");

                if(HAS_FLAG(ssa->flags, SSAFlags_InlineLiterals))
                {
                    ssa_print_function_inlined(func, reg);
                }
                else
                {
                    printf("%c%u = call %s(%c%u)\n", 
                           reg,
                           func->destination,
                           ssa_func_type_as_string(func->func),
                           reg,
                           mathsexpr_ssa_get_instruction_destination(func->argument));
                }

                break;
            }
            default:
                break;
        }
    }
}

void mathsexpr_ssa_destroy(SSA* ssa)
{
    mathsexpr_arena_destroy(&ssa->instructions_data);
    vector_free(ssa->instructions);
    hashmap_free(ssa->functions_lookup_table);
    free(ssa);
}