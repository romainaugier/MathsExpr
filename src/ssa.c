// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/ssa.h"

#include "libromano/math/common32.h"
#include "libromano/logger.h"
#include "libromano/hashmap.h"
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
        bool any_folding = false;

        for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
        {
            SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

            switch(instruction->type)
            {
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

                    break;
                }
            }
        }

        if(!any_folding)
        {
            break;
        }
    }

    return true;
}

#define INVALID_DESTINATION 0xFFFFFFFF

bool ssa_optimize_dead_code_elimination(SSA* ssa)
{
    const int value = 0;
    HashMap* destinations = hashmap_new(mathsexpr_ssa_num_instructions(ssa));

    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

        switch(instruction->type)
        {
            case SSAInstructionType_SSABinOP:
            {
                SSABinOP* binop = SSA_CAST(SSABinOP, instruction);

                uint32_t left_destination = ssa_get_destination(binop->left);
                uint32_t right_destination = ssa_get_destination(binop->right);

                hashmap_insert(destinations, 
                               (const void*)&left_destination,
                               sizeof(uint32_t),
                               (void*)&value,
                               sizeof(int));

                hashmap_insert(destinations, 
                               (const void*)&right_destination,
                               sizeof(uint32_t),
                               (void*)&value,
                               sizeof(int));

                break;
            }
            case SSAInstructionType_SSAUnOP:
            {
                SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction);

                uint32_t destination = ssa_get_destination(unop->operand);

                hashmap_insert(destinations, 
                               (const void*)&destination,
                               sizeof(uint32_t),
                               (void*)&value,
                               sizeof(int));

                break;
            }
            default:
                break;
        }
    }

    uint32_t i = 0;

    while(i < mathsexpr_ssa_num_instructions(ssa))
    {
        SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

        bool any_remove = false;

        switch(instruction->type)
        {
            case SSAInstructionType_SSALiteral:
            {
                const uint32_t destination = ssa_get_destination(instruction);
                void* found = hashmap_get(destinations, &destination, sizeof(uint32_t), NULL);

                if(found == NULL && destination != 0)
                {
                    vector_remove(ssa->instructions, i);
                    any_remove = true;
                }

                break;
            }
            case SSAInstructionType_SSAVariable:
            {
                const uint32_t destination = ssa_get_destination(instruction);
                void* found = hashmap_get(destinations, &destination, sizeof(uint32_t), NULL);

                if(found == NULL && destination != 0)
                {
                    vector_remove(ssa->instructions, i);
                    any_remove = true;
                }

                break;
            }
            default:
                break;
        }

        if(!any_remove)
        {
            i++;
        }
    }

    hashmap_free(destinations);

    return true;
}

#define SEED 0x12345678

uint32_t canonicalize_rhs(SSAInstruction* instruction)
{
    switch(instruction->type)
    {
        case SSAInstructionType_SSALiteral:
        {
            SSALiteral* lit = SSA_CAST(SSALiteral, instruction);
            return hash_murmur3((const void*)&lit->value, sizeof(float), SEED);
        }
        case SSAInstructionType_SSAVariable:
        {
            SSAVariable* var = SSA_CAST(SSAVariable, instruction);
            return (uint32_t)var->name;
        }
        case SSAInstructionType_SSABinOP:
        {
            SSABinOP* binop = SSA_CAST(SSABinOP, instruction);
            size_t operands = (size_t)binop->left + (size_t)binop->right + (size_t)binop->op;
            return hash_murmur3((const void*)&operands, sizeof(size_t), SEED);
        }
        case SSAInstructionType_SSAUnOP:
        {
            SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction);
            size_t operand = (size_t)unop->operand + (size_t)unop->op;
            return hash_murmur3((const void*)&operand, sizeof(size_t), SEED);
        }
        default:
            return 0;
    }
}

bool ssa_optimize_common_subexpression_elimination(SSA* ssa)
{
    while(true)
    {
        bool any_elimination = false;

        HashMap* expressions = hashmap_new(mathsexpr_ssa_num_instructions(ssa));
        HashMap* replacements = hashmap_new(mathsexpr_ssa_num_instructions(ssa));

        for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
        {
            SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

            size_t rhs = canonicalize_rhs(instruction);

            void* found = hashmap_get(expressions, &rhs, sizeof(size_t), NULL);

            if(found == NULL)
            {
                hashmap_insert(expressions, &rhs, sizeof(size_t), &instruction, sizeof(SSAInstruction*));
                continue;
            }

            hashmap_insert(replacements, &instruction, sizeof(SSAInstruction*), found, sizeof(SSAInstruction*));
        }

        HashMapIterator it = 0;
        void* key = NULL;
        uint32_t key_size = 0;
        void* value = NULL;
        uint32_t value_size = 0;

        while(hashmap_iterate(replacements, &it, &key, &key_size, &value, &value_size))
        {
            SSAInstruction* to_replace = *(SSAInstruction**)key;
            SSAInstruction* replacement = *(SSAInstruction**)value;

            for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
            {
                SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

                switch(instruction->type)
                {
                    case SSAInstructionType_SSABinOP:
                    {
                        SSABinOP* binop = SSA_CAST(SSABinOP, instruction);

                        if(binop->left == to_replace)
                        {
                            binop->left = replacement;
                            any_elimination = true;
                        }

                        if(binop->right == to_replace)
                        {
                            binop->right = replacement;
                            any_elimination = true;
                        }

                        break;
                    }
                    case SSAInstructionType_SSAUnOP:
                    {
                        SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction);

                        if(unop->operand == to_replace)
                        {
                            unop->operand = replacement;
                            any_elimination = true;
                        }

                        break;
                    }
                    default:
                        break;
                }
            }

            size_t pos = vector_find(ssa->instructions, &to_replace);

            if(pos == VECTOR_NOT_FOUND)
            {
                return false;
            }

            vector_remove(ssa->instructions, pos);
        }

        hashmap_free(expressions);
        hashmap_free(replacements);

        if(!any_elimination)
        {
            break;
        }
    }

    return true;
}

uint32_t ssa_find_earliest_instruction_use(SSA* ssa, SSAInstruction* instruction)
{
    uint32_t earliest_use = UINT32_MAX;

    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSAInstruction* instruction_at = mathsexpr_ssa_instruction_at(ssa, i);

        if(instruction == instruction_at)
        {
            continue;
        }

        switch(instruction_at->type)
        {
            case SSAInstructionType_SSABinOP:
            {
                SSABinOP* binop = SSA_CAST(SSABinOP, instruction_at);

                if(binop->left == instruction || binop->right == instruction)
                {
                    earliest_use = i < earliest_use ? i : earliest_use;
                }

                break;
            }
            case SSAInstructionType_SSAUnOP:
            {
                SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction_at);

                if(unop->operand == instruction)
                {
                    earliest_use = i < earliest_use ? i : earliest_use;
                }

                break;
            }
            
            default:
                break;
        }
    }

    return earliest_use;
}

bool ssa_optimize_code_sinking(SSA* ssa)
{
    while(true)
    {
        bool any_sinking = false;

        HashMap* earliest_uses = hashmap_new(mathsexpr_ssa_num_instructions(ssa));

        for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
        {
            SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

        }

        hashmap_free(earliest_uses);

        if(!any_sinking)
        {
            break;
        }
    }

    return true;
}

bool ssa_optimize_inline_literals(SSA* ssa)
{
    uint32_t i = 0;

    while(i < mathsexpr_ssa_num_instructions(ssa))
    {
        SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

        if(instruction->type == SSAInstructionType_SSALiteral &&
           ssa_get_destination(instruction) != 0)
        {
            vector_remove(ssa->instructions, i);
            continue;
        }

        i++;
    }

    SET_FLAG(ssa->flags, SSAFlags_InlineLiterals);

    return true;
}

bool mathsexpr_ssa_optimize(SSA* ssa)
{
    if(!ssa_optimize_constants_folding(ssa))
    {
        logger_log_error("Error during constant folding optimization");
        return false;
    }

    if(!ssa_optimize_dead_code_elimination(ssa))
    {
        logger_log_error("Error during dead code elimination optimization");
        return false;
    }

    if(!ssa_optimize_common_subexpression_elimination(ssa))
    {
        logger_log_error("Error during common subexpression elimination optimization");
        return false;
    }

    if(!ssa_optimize_inline_literals(ssa))
    {
        logger_log_error("Error during literals inlining optimization");
        return false;
    }

    if(!ssa_optimize_code_sinking(ssa))
    {
        logger_log_error("Error during code sinking optimization");
        return false;
    }

    return true;
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
        string_appendf(&fmt_string, "t%u", ssa_get_destination(binop->left));
    }

    string_appendf(&fmt_string, " %s ", ssa_binop_type_as_string(binop->op));

    if(binop->right->type == SSAInstructionType_SSALiteral)
    {
        SSALiteral* lit = SSA_CAST(SSALiteral, binop->right);
        string_appendf(&fmt_string, "%.3f", lit->value);
    }
    else
    {
        string_appendf(&fmt_string, "t%u", ssa_get_destination(binop->right));
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
                                                ssa_get_destination(binop->left),
                                                ssa_binop_type_as_string(binop->op),
                                                ssa_get_destination(binop->right));
                }

                break;
            }
            case SSAInstructionType_SSAUnOP:
            {
                SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction);
                MATHSEXPR_ASSERT(unop != NULL, "Wrong type casting");
                printf("t%u = %st%u\n", unop->destination, ssa_unop_type_as_string(unop->op), ssa_get_destination(unop->operand));
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