// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/ssa.h"

#include "libromano/math/common32.h"
#include "libromano/logger.h"

#include <string.h>

SSA* mathsexpr_ssa_new()
{
    SSA* new_ssa = (SSA*)malloc(sizeof(SSA));
    mathsexpr_arena_init(&new_ssa->instructions, 4096);

    new_ssa->num_instructions = 0;
    new_ssa->counter = 0;

    return new_ssa;
}

SSAInstruction* mathsexpr_ssa_new_literal(SSA* ssa, 
                                          float value, 
                                          uint32_t destination)
{
    SSALiteral lit = { SSAInstructionType_SSALiteral, value, destination };

    ssa->num_instructions++;

    return (SSAInstruction*)mathsexpr_arena_push(&ssa->instructions, &lit, sizeof(SSALiteral));
}

SSAInstruction* mathsexpr_ssa_new_variable(SSA* ssa,
                                           char name,
                                           uint32_t destination)
{
    SSAVariable var = { SSAInstructionType_SSAVariable, name, destination };

    ssa->num_instructions++;

    return (SSAInstruction*)mathsexpr_arena_push(&ssa->instructions, &var, sizeof(SSAVariable));
}

SSAInstruction* mathsexpr_ssa_new_binop(SSA* ssa, 
                                        SSABinOPType op, 
                                        SSAInstruction* left,
                                        SSAInstruction* right,
                                        uint32_t destination)
{
    SSABinOP binop = { SSAInstructionType_SSABinOP, op, left, right, destination };

    ssa->num_instructions++;

    return (SSAInstruction*)mathsexpr_arena_push(&ssa->instructions, &binop, sizeof(SSABinOP));
}

SSAInstruction* mathsexpr_ssa_new_unop(SSA* ssa,
                                       SSAUnOPType op,
                                       SSAInstruction* operand,
                                       uint32_t destination)
{
    SSAUnOP unop = { SSAInstructionType_SSAUnOP, op, operand, destination };

    ssa->num_instructions++;

    return (SSAInstruction*)mathsexpr_arena_push(&ssa->instructions, &unop, sizeof(SSAUnOP));
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

uint32_t ssa_get_destination(SSAInstruction* instruction)
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

bool ssa_optimize_constants_folding(SSA* ssa)
{
    while(true)
    {
        size_t offset = 0;
        bool any_folding = false;
        uint32_t i = 0;
        
        while(i < ssa->num_instructions)
        {
            SSAInstruction* instruction = mathsexpr_arena_at(&ssa->instructions, offset);

            switch(instruction->type)
            {
                case SSAInstructionType_SSALiteral:
                    offset += sizeof(SSALiteral);
                    i++;
                    break;
                case SSAInstructionType_SSAVariable:
                    offset += sizeof(SSAVariable);
                    i++;
                    break;
                case SSAInstructionType_SSAUnOP:
                    offset += sizeof(SSAUnOP);
                    i++;
                    break;
                case SSAInstructionType_SSABinOP:
                {
                    SSABinOP* binop = SSA_CAST(SSABinOP, instruction);
                    MATHSEXPR_ASSERT(binop != NULL, "Wrong type casting, should be SSABinOP");

                    if(binop->left->type == SSAInstructionType_SSALiteral && 
                       binop->right->type == SSAInstructionType_SSALiteral)
                    {
                        SSALiteral* lit_a = SSA_CAST(SSALiteral, binop->left);
                        MATHSEXPR_ASSERT(lit_a != NULL, "Wrong type casting, should be SSALiteral");

                        SSALiteral* lit_b = SSA_CAST(SSALiteral, binop->right);
                        MATHSEXPR_ASSERT(lit_b != NULL, "Wrong type casting, should be SSALiteral");

                        SSABinOPType op = binop->op;
                        uint32_t destination = binop->destination;

                        memset(instruction, 0, sizeof(SSABinOP));
                        instruction->type = SSAInstructionType_SSALiteral;

                        SSALiteral* result = SSA_CAST(SSALiteral, instruction);
                        result->destination = destination;

                        switch(op)
                        {
                            case SSABinOPType_Add:
                                result->value = lit_a->value + lit_b->value;
                                break;
                            case SSABinOPType_Sub:
                                result->value = lit_a->value - lit_b->value;
                                break;
                            case SSABinOPType_Mul:
                                result->value = lit_a->value * lit_b->value;
                                break;
                            case SSABinOPType_Div:
                                result->value = lit_a->value / lit_b->value;
                                break;
                            case SSABinOPType_Mod:
                                result->value = mathf_fmod(lit_a->value, lit_b->value);
                                break;
                            case SSABinOPType_Pow:
                                result->value = mathf_pow(lit_a->value, lit_b->value);
                                break;
                            default:
                                result->value = 0.0f;
                                break;
                        }

                        any_folding = true;
                    }

                    offset += sizeof(SSABinOP);
                    i++;

                    break;
                }
                default:
                    /* We are in a zeroed zone after a binop has been constant folded */
                    offset += 1;
                    break;
            }
        }

        if(!any_folding)
        {
            break;
        }
    }

    return true;
}

bool mathsexpr_ssa_optimize(SSA* ssa)
{
    if(!ssa_optimize_constants_folding(ssa))
    {
        logger_log_error("Error during constant folding optimization");
        return false;
    }

    return true;
}

void mathsexpr_ssa_print(SSA* ssa)
{
    size_t offset = 0;
    uint32_t i = 0;

    while(i < ssa->num_instructions)
    {
        SSAInstruction* instruction = mathsexpr_arena_at(&ssa->instructions, offset);

        switch(instruction->type)
        {
            case SSAInstructionType_SSALiteral:
            {
                SSALiteral* lit = SSA_CAST(SSALiteral, instruction);
                MATHSEXPR_ASSERT(lit != NULL, "Wrong type casting");
                printf("t%u = %.3f\n", lit->destination, lit->value);
                offset += sizeof(SSALiteral);
                i++;
                break;
            }
            case SSAInstructionType_SSAVariable:
            {
                SSAVariable* var = SSA_CAST(SSAVariable, instruction);
                MATHSEXPR_ASSERT(var != NULL, "Wrong type casting");
                printf("t%u = %c\n", var->destination, var->name);
                offset += sizeof(SSAVariable);
                i++;
                break;
            }
            case SSAInstructionType_SSABinOP:
            {
                SSABinOP* binop = SSA_CAST(SSABinOP, instruction);
                MATHSEXPR_ASSERT(binop != NULL, "Wrong type casting");
                printf("t%u = t%u %s t%u\n", binop->destination, 
                                             ssa_get_destination(binop->left),
                                             ssa_binop_type_as_string(binop->op),
                                             ssa_get_destination(binop->right));
                offset += sizeof(SSABinOP);
                i++;
                break;
            }
            case SSAInstructionType_SSAUnOP:
            {
                SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction);
                MATHSEXPR_ASSERT(unop != NULL, "Wrong type casting");
                printf("t%u = %st%u\n", ssa_unop_type_as_string(unop->op), ssa_get_destination(unop->operand));
                offset += sizeof(SSAUnOP);
                i++;
                break;
            }
            default:
                /* We are in a zeroed zone after a binop has been constant folded */
                offset += 1;
                break;
        }
    }
}

void mathsexpr_ssa_destroy(SSA* ssa)
{
    mathsexpr_arena_destroy(&ssa->instructions);
    free(ssa);
}