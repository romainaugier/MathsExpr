// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/ssa_register_alloc.h"

#include "libromano/flag.h"
#include "libromano/logger.h"

void mathsexpr_ssa_get_live_intervals(SSA* ssa, Vector* intervals)
{
    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSALiveInterval interval = { i, 0, 0 };
        vector_push_back(intervals, &interval);
    }

    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);
        
        switch(instruction->type)
        {
            case SSAInstructionType_SSAVariable:
            {
                SSAVariable* var = SSA_CAST(SSAVariable, instruction);
                MATHSEXPR_ASSERT(var != NULL, "Wrong casting, should be SSAVariable");

                uint32_t destination = var->destination;

                SSALiveInterval* var_interval = (SSALiveInterval*)vector_at(intervals, destination);

                var_interval->start = var_interval->start == 0 ? (i + 1) : var_interval->start;
                var_interval->end = var_interval->end < (i + 1) ? (i + 1) : var_interval->end;

                break;
            }

            case SSAInstructionType_SSABinOP:
            {
                SSABinOP* binop = SSA_CAST(SSABinOP, instruction);
                MATHSEXPR_ASSERT(binop != NULL, "Wrong casting, should be SSABinOP");

                uint32_t destination = binop->destination;

                SSALiveInterval* binop_interval = (SSALiveInterval*)vector_at(intervals, destination);

                binop_interval->start = binop_interval->start == 0 ? (i + 1) : binop_interval->start;
                binop_interval->end = binop_interval->end < (i + 1) ? (i + 1) : binop_interval->end;

                if(binop->left->type != SSAInstructionType_SSALiteral || !HAS_FLAG(ssa->flags, SSAFlags_InlineLiterals))
                {
                    uint32_t left_destination = mathsexpr_ssa_get_instruction_destination(binop->left);
                    SSALiveInterval* left_interval = (SSALiveInterval*)vector_at(intervals, left_destination);
                    left_interval->start = left_interval->start == 0 ? (i + 1) : left_interval->start;
                    left_interval->end = left_interval->end < (i + 1) ? (i + 1) : left_interval->end;
                }

                if(binop->right->type != SSAInstructionType_SSALiteral || !HAS_FLAG(ssa->flags, SSAFlags_InlineLiterals))
                {
                    uint32_t right_destination = mathsexpr_ssa_get_instruction_destination(binop->right);
                    SSALiveInterval* right_interval = (SSALiveInterval*)vector_at(intervals, right_destination);
                    right_interval->start = right_interval->start == 0 ? (i + 1) : right_interval->start;
                    right_interval->end = right_interval->end < (i + 1) ? (i + 1) : right_interval->end;
                }

                break;
            }

            case SSAInstructionType_SSAUnOP:
            {
                SSAUnOP* unop = SSA_CAST(SSAUnOP, instruction);
                MATHSEXPR_ASSERT(unop != NULL, "Wrong casting, should be SSAUnOP");

                uint32_t destination = unop->destination;

                SSALiveInterval* unop_interval = (SSALiveInterval*)vector_at(intervals, destination);

                unop_interval->start = unop_interval->start == 0 ? (i + 1) : unop_interval->start;
                unop_interval->end = unop_interval->end < (i + 1) ? (i + 1) : unop_interval->end;

                uint32_t operand_destination = mathsexpr_ssa_get_instruction_destination(unop->operand);
                SSALiveInterval* unop_operand_interval = (SSALiveInterval*)vector_at(intervals, operand_destination);
                unop_operand_interval->start = unop_operand_interval->start == 0 ? (i + 1) : unop_operand_interval->start;
                unop_operand_interval->end = unop_operand_interval->end < (i + 1) ? (i + 1) : unop_operand_interval->end;

                break;
            }

            case SSAInstructionType_SSAFunction:
            {
                SSAFunction* func = SSA_CAST(SSAFunction, instruction);
                MATHSEXPR_ASSERT(func != NULL, "Wrong casting, should be SSAFunction");

                uint32_t destination = func->destination;

                SSALiveInterval* unop_interval = (SSALiveInterval*)vector_at(intervals, destination);

                unop_interval->start = unop_interval->start == 0 ? (i + 1) : unop_interval->start;
                unop_interval->end = unop_interval->end < (i + 1) ? (i + 1) : unop_interval->end;

                uint32_t argument_destination = mathsexpr_ssa_get_instruction_destination(func->argument);

                SSALiveInterval* func_argument_interval = (SSALiveInterval*)vector_at(intervals, argument_destination);

                func_argument_interval->start = func_argument_interval->start == 0 ? (i + 1) : func_argument_interval->start;
                func_argument_interval->end = func_argument_interval->end < (i + 1) ? (i + 1) : func_argument_interval->end;

                break;
            }
            
            default:
                break;
        }
    }
}

void mathsexpr_ssa_print_live_intervals(Vector* intervals)
{
    for(uint32_t i = 0; i < vector_size(intervals); i++)
    {
        SSALiveInterval* interval = (SSALiveInterval*)vector_at(intervals, i);
        printf("t%u (%u-%u)\n", interval->destination, (uint32_t)interval->start, (uint32_t)interval->end);
    }
}

int ssa_compare_live_intervals(const void* a, const void* b)
{
    SSALiveInterval* ia = (SSALiveInterval*)a;
    SSALiveInterval* ib = (SSALiveInterval*)b;

    return ia->end - ib->end;
}

void ssa_find_preassigned_registers(SSA* ssa, HashMap* preassigned_registers)
{
    const uint32_t register_zero = 0;

    uint32_t variables_order_of_appearance = 0;

    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

        switch(instruction->type)
        {
            case SSAInstructionType_SSAVariable:
            {
                hashmap_insert(preassigned_registers,
                               &instruction,
                               sizeof(SSAInstruction*),
                               &variables_order_of_appearance,
                               sizeof(uint32_t));

                variables_order_of_appearance++;

                break;
            }

            case SSAInstructionType_SSAFunction:
            {
                SSAFunction* func = SSA_CAST(SSAFunction, instruction);

                hashmap_insert(preassigned_registers,
                               &instruction,
                               sizeof(SSAInstruction*),
                               &register_zero,
                               sizeof(uint32_t));

                hashmap_insert(preassigned_registers,
                               &func->argument,
                               sizeof(SSAInstruction*),
                               &register_zero,
                               sizeof(uint32_t));

                break;
            }

            default:
                break;
        }
    }

    SSAInstruction* last_instruction = mathsexpr_ssa_instruction_at(ssa,
                                                                    mathsexpr_ssa_num_instructions(ssa) - 1);

    hashmap_insert(preassigned_registers,
                   &last_instruction,
                   sizeof(SSAInstruction*),
                   &register_zero,
                   sizeof(uint32_t));
}

bool mathsexpr_ssa_allocate_registers(SSA* ssa)
{
    if(HAS_FLAG(ssa->flags, SSAFlags_HasRegistersAsDestination))
    {
        logger_log_error("SSA has already been allocated registers");
        return false;
    }

    HashMap* preassigned_registers = hashmap_new(mathsexpr_ssa_num_instructions(ssa));
    Vector* live_intervals = vector_new(mathsexpr_ssa_num_instructions(ssa), sizeof(SSALiveInterval));

    ssa_find_preassigned_registers(ssa, preassigned_registers);
    mathsexpr_ssa_get_live_intervals(ssa, live_intervals);

    vector_free(live_intervals);
    hashmap_free(preassigned_registers);

    SET_FLAG(ssa->flags, SSAFlags_HasRegistersAsDestination);

    return true;
}