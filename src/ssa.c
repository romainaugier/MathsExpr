// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/ssa.h"

#include "libromano/logger.h"
#include "libromano/string.h"
#include "libromano/flag.h"

#include <string.h>

SSA* mathsexpr_ssa_new()
{
    SSA* new_ssa = (SSA*)malloc(sizeof(SSA));
    mathsexpr_arena_init(&new_ssa->instructions_data, 4096);

    new_ssa->instructions = vector_new(128, sizeof(SSAInstruction*));
    new_ssa->counter = 0;

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
                                           char name,
                                           uint32_t destination)
{
    SSAVariable var = { SSAInstructionType_SSAVariable, name, destination };

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

            return mathsexpr_ssa_new_variable(ssa, var->name, destination);
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

uint32_t mathsexpr_ssa_get_instruction_destination(SSAInstruction* instruction)
{
    switch(instruction->type)
    {
        case SSAInstructionType_SSALiteral:
        {
            SSALiteral* lit = SSA_CAST(SSALiteral, instruction);
            MATHSEXPR_ASSERT(lit != NULL, "Wrong type casting, should be SSALiteral");
            return lit->destination;
        }
        case SSAInstructionType_SSAVariable:
        {
            SSAVariable* var = SSA_CAST(SSAVariable, instruction);
            MATHSEXPR_ASSERT(var != NULL, "Wrong type casting, should be SSAVariable");
            return var->destination;
        }
        case SSAInstructionType_SSABinOP:
        {
            SSABinOP* binop = SSA_CAST(SSABinOP, instruction);
            MATHSEXPR_ASSERT(binop != NULL, "Wrong type casting, should be SSABinOP");
            return binop->destination;
        }
        case SSAInstructionType_SSAUnOP:
        {
            SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction);
            MATHSEXPR_ASSERT(unop != NULL, "Wrong type casting, should be SSAUnOP");
            return unop->destination;
        }
        default:
            return 0;
    }
}

void ssa_print_binop_inlined(SSABinOP* binop)
{
    String fmt_string = string_newf("t%d = ", binop->destination);

    if(binop->left->type == SSAInstructionType_SSALiteral)
    {
        SSALiteral* lit = SSA_CAST(SSALiteral, binop->left);
        string_appendf(&fmt_string, "%.3f", lit->value);
    }
    else
    {
        string_appendf(&fmt_string, "t%u", mathsexpr_ssa_get_instruction_destination(binop->left));
    }

    string_appendf(&fmt_string, " %s ", ssa_binop_type_as_string(binop->op));

    if(binop->right->type == SSAInstructionType_SSALiteral)
    {
        SSALiteral* lit = SSA_CAST(SSALiteral, binop->right);
        string_appendf(&fmt_string, "%.3f", lit->value);
    }
    else
    {
        string_appendf(&fmt_string, "t%u", mathsexpr_ssa_get_instruction_destination(binop->right));
    }

    puts(fmt_string); 

    string_free(fmt_string);
}

void mathsexpr_ssa_print(SSA* ssa)
{
    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

        switch(instruction->type)
        {
            case SSAInstructionType_SSALiteral:
            {
                SSALiteral* lit = SSA_CAST(SSALiteral, instruction);
                MATHSEXPR_ASSERT(lit != NULL, "Wrong type casting");
                printf("t%u = %.3f\n", lit->destination, lit->value);
                break;
            }
            case SSAInstructionType_SSAVariable:
            {
                SSAVariable* var = SSA_CAST(SSAVariable, instruction);
                MATHSEXPR_ASSERT(var != NULL, "Wrong type casting");
                printf("t%u = %c\n", var->destination, var->name);
                break;
            }
            case SSAInstructionType_SSABinOP:
            {
                SSABinOP* binop = SSA_CAST(SSABinOP, instruction);
                MATHSEXPR_ASSERT(binop != NULL, "Wrong type casting");

                if(HAS_FLAG(ssa->flags, SSAFlags_InlineLiterals))
                {
                    ssa_print_binop_inlined(binop);
                }
                else
                {
                    printf("t%u = t%u %s t%u\n", binop->destination, 
                                                mathsexpr_ssa_get_instruction_destination(binop->left),
                                                ssa_binop_type_as_string(binop->op),
                                                mathsexpr_ssa_get_instruction_destination(binop->right));
                }

                break;
            }
            case SSAInstructionType_SSAUnOP:
            {
                SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction);
                MATHSEXPR_ASSERT(unop != NULL, "Wrong type casting");
                printf("t%u = %st%u\n", unop->destination, ssa_unop_type_as_string(unop->op), mathsexpr_ssa_get_instruction_destination(unop->operand));
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
    free(ssa);
}