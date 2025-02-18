// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/ssa_optimize.h"

#include "libromano/math/common32.h"
#include "libromano/hashmap.h"
#include "libromano/flag.h"
#include "libromano/logger.h"

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

                case SSAInstructionType_SSAFunction:
                {
                    SSAFunction* func = SSA_CAST(SSAFunction, instruction);
                    MATHSEXPR_ASSERT(func != NULL, "Wrong type casting, should be SSAFunction");

                    if(func->argument->type == SSAInstructionType_SSALiteral)
                    {
                        SSALiteral* lit = SSA_CAST(SSALiteral, func->argument);
                        MATHSEXPR_ASSERT(lit != NULL, "Wrong type casting, should be SSALiteral");

                        SSAFuncType func_type = func->func;
                        uint32_t destination = func->destination;

                        memset(instruction, 0, sizeof(SSAFunction));
                        instruction->type = SSAInstructionType_SSALiteral;

                        SSALiteral* result = SSA_CAST(SSALiteral, instruction);
                        result->destination = destination;

                        switch(func_type)
                        {
                            case SSAFuncType_Abs:
                                result->value = mathf_abs(lit->value);
                                break;
                            case SSAFuncType_Exp:
                                result->value = mathf_exp(lit->value);
                                break;
                            case SSAFuncType_Sqrt:
                                result->value = mathf_sqrt(lit->value);
                                break;
                            case SSAFuncType_Rcp:
                                result->value = mathf_rcp(lit->value);
                                break;
                            case SSAFuncType_Rsqrt:
                                result->value = mathf_rsqrt(lit->value);
                                break;
                            case SSAFuncType_Log:
                                result->value = mathf_log(lit->value);
                                break;
                            case SSAFuncType_Log2:
                                result->value = mathf_log2(lit->value);
                                break;
                            case SSAFuncType_Log10:
                                result->value = mathf_log10(lit->value);
                                break;
                            case SSAFuncType_Floor:
                                result->value = mathf_floor(lit->value);
                                break;
                            case SSAFuncType_Ceil:
                                result->value = mathf_ceil(lit->value);
                                break;
                            case SSAFuncType_Frac:
                                result->value = mathf_frac(lit->value);
                                break;
                            case SSAFuncType_Acos:
                                result->value = mathf_acos(lit->value);
                                break;
                            case SSAFuncType_Asin:
                                result->value = mathf_asin(lit->value);
                                break;
                            case SSAFuncType_Atan:
                                result->value = mathf_atan(lit->value);
                                break;
                            case SSAFuncType_Cos:
                                result->value = mathf_cos(lit->value);
                                break;
                            case SSAFuncType_Sin:
                                result->value = mathf_sin(lit->value);
                                break;
                            case SSAFuncType_Tan:
                                result->value = mathf_tan(lit->value);
                                break;
                            case SSAFuncType_Cosh:
                                result->value = mathf_cosh(lit->value);
                                break;
                            case SSAFuncType_Sinh:
                                result->value = mathf_sinh(lit->value);
                                break;
                            case SSAFuncType_Tanh:
                                result->value = mathf_tanh(lit->value);
                                break;
                            default:
                                result->value = 0.0f;
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
                MATHSEXPR_ASSERT(binop != NULL, "Wrong casting, should be SSABinOP");

                uint32_t left_destination = mathsexpr_ssa_get_instruction_destination(binop->left);
                uint32_t right_destination = mathsexpr_ssa_get_instruction_destination(binop->right);

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
                MATHSEXPR_ASSERT(unop != NULL, "Wrong casting, should be SSAUnOP");

                uint32_t destination = mathsexpr_ssa_get_instruction_destination(unop->operand);

                hashmap_insert(destinations, 
                               (const void*)&destination,
                               sizeof(uint32_t),
                               (void*)&value,
                               sizeof(int));

                break;
            }

            case SSAInstructionType_SSAFunction:
            {
                SSAFunction* func = SSA_CAST(SSAFunction, instruction);
                MATHSEXPR_ASSERT(func != NULL, "Wrong casting, should be SSAFunction");

                uint32_t destination = mathsexpr_ssa_get_instruction_destination(func->argument);

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
                const uint32_t destination = mathsexpr_ssa_get_instruction_destination(instruction);
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
                const uint32_t destination = mathsexpr_ssa_get_instruction_destination(instruction);
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
            MATHSEXPR_ASSERT(lit != NULL, "Wrong casting, should be SSALiteral");
            return hash_murmur3((const void*)&lit->value, sizeof(float), SEED);
        }
        case SSAInstructionType_SSAVariable:
        {
            SSAVariable* var = SSA_CAST(SSAVariable, instruction);
            MATHSEXPR_ASSERT(var != NULL, "Wrong casting, should be SSAVariable");
            return hash_murmur3(var->name, var->name_length, SEED);
        }
        case SSAInstructionType_SSABinOP:
        {
            SSABinOP* binop = SSA_CAST(SSABinOP, instruction);
            MATHSEXPR_ASSERT(binop != NULL, "Wrong casting, should be SSABinOP");
            uint32_t left_hash = canonicalize_rhs(binop->left);
            uint32_t right_hash = canonicalize_rhs(binop->right);
            uint32_t parts[3] = { left_hash, right_hash, (uint32_t)binop->op };
            return hash_murmur3((const void*)parts, sizeof(parts), SEED);
        }
        case SSAInstructionType_SSAUnOP:
        {
            SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction);
            MATHSEXPR_ASSERT(unop != NULL, "Wrong casting, should be SSAUnOP");
            uint32_t operand_hash = canonicalize_rhs(unop->operand);
            uint32_t parts[2] = { operand_hash, (uint32_t)unop->op };
            return hash_murmur3((const void*)parts, sizeof(parts), SEED);
        }
        case SSAInstructionType_SSAFunction:
        {
            SSAFunction* func = SSA_CAST(SSAFunction, instruction);
            MATHSEXPR_ASSERT(func != NULL, "Wrong casting, should be SSAFunction");
            uint32_t argument_hash = canonicalize_rhs(func->argument);
            uint32_t parts[2] = { argument_hash, (uint32_t)func->func };
            return hash_murmur3((const void*)parts, sizeof(parts), SEED);
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
                        MATHSEXPR_ASSERT(binop != NULL, "Wrong casting, should be SSABinOP");

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
                        MATHSEXPR_ASSERT(unop != NULL, "Wrong casting, should be SSAUnOP");

                        if(unop->operand == to_replace)
                        {
                            unop->operand = replacement;
                            any_elimination = true;
                        }

                        break;
                    }

                    case SSAInstructionType_SSAFunction:
                    {
                        SSAFunction* func = SSA_CAST(SSAFunction, instruction);
                        MATHSEXPR_ASSERT(func != NULL, "Wrong casting, should be SSAFunction");

                        if(func->argument == to_replace)
                        {
                            func->argument = replacement;
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

#define DEAD_INSTRUCTION UINT32_MAX

uint32_t ssa_find_earliest_instruction_use(SSA* ssa, SSAInstruction* instruction)
{
    uint32_t earliest_use = DEAD_INSTRUCTION;

    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSAInstruction* current_instr = mathsexpr_ssa_instruction_at(ssa, i);

        if(current_instr == instruction) 
        {
            continue;
        }

        switch(current_instr->type)
        {
            case SSAInstructionType_SSABinOP:
            {
                SSABinOP* binop = SSA_CAST(SSABinOP, current_instr);
                MATHSEXPR_ASSERT(binop != NULL, "Wrong casting, should be SSABinOP");

                if(binop->left == instruction || binop->right == instruction) 
                {
                    earliest_use = i < earliest_use ? i : earliest_use;
                }

                break;
            }

            case SSAInstructionType_SSAUnOP:
            {
                SSAUnOP* unop = SSA_CAST(SSAUnOP, current_instr);
                MATHSEXPR_ASSERT(unop != NULL, "Wrong casting, should be SSAUnOP");

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
    return true;

    /*
    while(true)
    {
        bool any_sinking = false;

        for(int32_t i = mathsexpr_ssa_num_instructions(ssa) - 1; i >= 0; i--)
        {
            SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

            uint32_t earliest_use = ssa_find_earliest_instruction_use(ssa, instruction);

            if(earliest_use == DEAD_INSTRUCTION)
            {
                continue;
            }

            if(earliest_use > 0 && earliest_use > ((uint32_t)i + 1UL))
            {
                uint32_t new_pos = earliest_use - 1;

                vector_remove(ssa->instructions, i);
                vector_insert(ssa->instructions, &instruction, new_pos);

                any_sinking = true;
            }
        }

        if(!any_sinking)
        {
            break;
        }
    }

    return true;
    */
}

bool ssa_optimize_inline_literals(SSA* ssa)
{
    uint32_t i = 0;

    while(i < mathsexpr_ssa_num_instructions(ssa))
    {
        SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

        if(instruction->type == SSAInstructionType_SSALiteral &&
           mathsexpr_ssa_get_instruction_destination(instruction) != 0)
        {
            vector_remove(ssa->instructions, i);
            continue;
        }

        i++;
    }

    SET_FLAG(ssa->flags, SSAFlags_InlineLiterals);

    return true;
}

bool ssa_optimize_strength_reduction(SSA* ssa)
{
    return true;
}

bool ssa_optimize_refine_destinations(SSA* ssa)
{
    uint32_t new_counter = 0;
    HashMap* dest_map = hashmap_new(mathsexpr_ssa_num_instructions(ssa));

    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSAInstruction* instr = mathsexpr_ssa_instruction_at(ssa, i);
        
        if(instr->type == SSAInstructionType_SSALiteral)
        {
            continue;
        }

        uint32_t old_dest = mathsexpr_ssa_get_instruction_destination(instr);

        hashmap_insert(dest_map, 
                      (const void*)&old_dest, sizeof(uint32_t),
                      (void*)&new_counter, sizeof(uint32_t));

        new_counter++;
    }

    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSAInstruction* instr = mathsexpr_ssa_instruction_at(ssa, i);

        if(instr->type != SSAInstructionType_SSALiteral)
        {
            uint32_t old_dest = mathsexpr_ssa_get_instruction_destination(instr);
            uint32_t* new_dest = hashmap_get(dest_map, &old_dest, sizeof(uint32_t), NULL);
            
            switch(instr->type)
            {
                case SSAInstructionType_SSAVariable:
                {
                    SSAVariable* var = SSA_CAST(SSAVariable, instr);
                    MATHSEXPR_ASSERT(var != NULL, "Wrong casting, should be SSAVariable");
                    var->destination = *new_dest;
                    break;
                }
                case SSAInstructionType_SSABinOP:
                    SSABinOP* binop = SSA_CAST(SSABinOP, instr);
                    MATHSEXPR_ASSERT(binop != NULL, "Wrong casting, should be SSABinOP");
                    binop->destination = *new_dest;
                    break;
                case SSAInstructionType_SSAUnOP:
                    SSAUnOP* unop = SSA_CAST(SSAUnOP, instr);
                    MATHSEXPR_ASSERT(unop != NULL, "Wrong casting, should be SSAUnOP");
                    unop->destination = *new_dest;
                    break;
                case SSAInstructionType_SSAFunction:
                    SSAFunction* func = SSA_CAST(SSAFunction, instr);
                    MATHSEXPR_ASSERT(func != NULL, "Wrong casting, should be SSAFunction");
                    func->destination = *new_dest;
                    break;
                default: 
                    break;
            }
        }
    }

    ssa->counter = new_counter;
    hashmap_free(dest_map);

    return true;
}

bool mathsexpr_ssa_optimize(SSA* ssa, uint64_t optimization_flags)
{
    const uint32_t n_passes = HAS_FLAG(optimization_flags, SSAOptimizationFlags_MultiPass) ? 2 : 1;

    for(uint32_t i = 0; i < n_passes; i++)
    {
        if(HAS_FLAG(optimization_flags, SSAOptimizationFlags_ConstantsFolding) && 
        !ssa_optimize_constants_folding(ssa))
        {
            logger_log_error("Error during constant folding optimization");
            return false;
        }

        if(HAS_FLAG(optimization_flags, SSAOptimizationFlags_DeadCodeElimination) && 
        !ssa_optimize_dead_code_elimination(ssa))
        {
            logger_log_error("Error during dead code elimination optimization");
            return false;
        }

        if(HAS_FLAG(optimization_flags, SSAOptimizationFlags_CommonSubexpressionElimination) && 
        !ssa_optimize_common_subexpression_elimination(ssa))
        {
            logger_log_error("Error during common subexpression elimination optimization");
            return false;
        }

        if(HAS_FLAG(optimization_flags, SSAOptimizationFlags_InlineLiterals) && 
        !ssa_optimize_inline_literals(ssa))
        {
            logger_log_error("Error during literals inlining optimization");
            return false;
        }

        if(HAS_FLAG(optimization_flags, SSAOptimizationFlags_CodeSinking) && 
        !ssa_optimize_code_sinking(ssa))
        {
            logger_log_error("Error during code sinking optimization");
            return false;
        }

        if(HAS_FLAG(optimization_flags, SSAOptimizationFlags_StrengthReduction) &&
        !ssa_optimize_strength_reduction(ssa))
        {
            logger_log_error("Error during strength reduction optimization");
            return false;
        }
    }

    if(!ssa_optimize_refine_destinations(ssa))
    {
        logger_log_error("Error during destination refinement optimization");
        return false;
    }

    return true;
}
