// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/ssa_register_alloc.h"

#include "libromano/flag.h"
#include "libromano/bit.h"
#include "libromano/logger.h"

void mathsexpr_ssa_get_live_intervals(SSA* ssa, Vector* intervals)
{
    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSALiveInterval interval = { mathsexpr_ssa_instruction_at(ssa, i), NO_REG, 0, 0, 0 };
        vector_push_back(intervals, &interval);
    }

    uint32_t variables_order_of_appearance = 0;

    for(int32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);
        
        switch(instruction->type)
        {
            case SSAInstructionType_SSAVariable:
            {
                SSAVariable* var = SSA_CAST(SSAVariable, instruction);
                MATHSEXPR_ASSERT(var != NULL, "Wrong type casting, should be SSAVariable");

                uint32_t destination = var->destination;

                SSALiveInterval* var_interval = (SSALiveInterval*)vector_at(intervals, destination);

                var_interval->start = 1;
                var_interval->end = var_interval->end < (i + 1) ? (i + 1) : var_interval->end;
                var_interval->reg = variables_order_of_appearance;

                variables_order_of_appearance++;

                break;
            }

            case SSAInstructionType_SSABinOP:
            {
                SSABinOP* binop = SSA_CAST(SSABinOP, instruction);
                MATHSEXPR_ASSERT(binop != NULL, "Wrong type casting, should be SSABinOP");

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
                MATHSEXPR_ASSERT(unop != NULL, "Wrong type casting, should be SSAUnOP");

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
                MATHSEXPR_ASSERT(func != NULL, "Wrong type casting, should be SSAFunction");

                uint32_t destination = func->destination;

                SSALiveInterval* func_interval = (SSALiveInterval*)vector_at(intervals, destination);

                func_interval->start = func_interval->start == 0 ? (i + 1) : func_interval->start;
                func_interval->end = func_interval->end < (i + 1) ? (i + 1) : func_interval->end;
                func_interval->reg = 0; 

                uint32_t argument_destination = mathsexpr_ssa_get_instruction_destination(func->argument);

                SSALiveInterval* func_argument_interval = (SSALiveInterval*)vector_at(intervals, argument_destination);

                func_argument_interval->start = func_argument_interval->start == 0 ? (i + 1) : func_argument_interval->start;
                func_argument_interval->end = func_argument_interval->end < (i + 1) ? (i + 1) : func_argument_interval->end;
                func_argument_interval->reg = 0; 

                break;
            }
            
            default:
                break;
        }
    }

    SSALiveInterval* var_interval = (SSALiveInterval*)vector_back(intervals);
    var_interval->reg = 0;
}

void mathsexpr_ssa_print_live_intervals(Vector* intervals)
{
    for(uint32_t i = 0; i < vector_size(intervals); i++)
    {
        SSALiveInterval* interval = (SSALiveInterval*)vector_at(intervals, i);
        printf("t%u (%u-%u)",
               mathsexpr_ssa_get_instruction_destination(interval->instruction),
               (uint32_t)interval->start, (uint32_t)interval->end);

        if(interval->reg != NO_REG)
        {
            printf(" preassigned: %u\n", interval->reg);
        }
        else
        {
            printf("\n");
        }
    }
}

int ssa_compare_live_intervals(const void* a, const void* b)
{
    SSALiveInterval* ia = (SSALiveInterval*)a;
    SSALiveInterval* ib = (SSALiveInterval*)b;

    if(ia->reg != NO_REG)
    {
        if(ib->reg != NO_REG)
        {
            return 0;
        }

        return -1;
    }
    else if(ib->reg != NO_REG)
    {
        return 1;
    }

    return ia->end - ib->end;
}

SSALiveInterval* ssa_get_operand_interval(SSA* ssa, SSAInstruction* instr, Vector* intervals) 
{
    if(instr->type == SSAInstructionType_SSALiteral && HAS_FLAG(ssa->flags, SSAFlags_InlineLiterals)) 
    {
        return NULL;
    }

    for(uint32_t i = 0; i < vector_size(intervals); i++)
    {
        SSALiveInterval* interval = (SSALiveInterval*)vector_at(intervals, i);

        if(interval->instruction == instr)
        {
            return interval;
        }
    }

    return NULL;
}

void ssa_find_last_uses(SSA* ssa, Vector* intervals)
{
    for(int32_t i = mathsexpr_ssa_num_instructions(ssa) - 1; i >= 0; i--)
    {
        SSAInstruction* instr = mathsexpr_ssa_instruction_at(ssa, i);
        
        if(instr->type == SSAInstructionType_SSABinOP) 
        {
            SSABinOP* binop = SSA_CAST(SSABinOP, instr);
            MATHSEXPR_ASSERT(binop != NULL, "Wrong type casting, should be SSABinOP");
            
            if(!HAS_FLAG(ssa->flags, SSAFlags_InlineLiterals) || binop->left->type != SSAInstructionType_SSALiteral) 
            {
                SSALiveInterval* left = ssa_get_operand_interval(ssa, binop->left, intervals);
                left->last_use = left->last_use == 0 ? i : left->last_use;
            }
            
            if(!HAS_FLAG(ssa->flags, SSAFlags_InlineLiterals) || binop->right->type != SSAInstructionType_SSALiteral)
            {
                SSALiveInterval* right = ssa_get_operand_interval(ssa, binop->right, intervals);
                right->last_use = right->last_use == 0 ? i : right->last_use;
            }
        }
    }
}

typedef struct {
    uint32_t first;
    uint32_t second;
} ConflictGroup;

bool mathsexpr_ssa_allocate_registers(SSA* ssa, uint32_t num_registers)
{
    if(num_registers > 64)
    {
        logger_log_error("Cannot allocate to more than 64 registers");
        return false;
    }

    if(HAS_FLAG(ssa->flags, SSAFlags_HasRegistersAsDestination))
    {
        logger_log_error("SSA has already been allocated registers");
        return false;
    }

    Vector* live_intervals = vector_new(mathsexpr_ssa_num_instructions(ssa), sizeof(SSALiveInterval));
    mathsexpr_ssa_get_live_intervals(ssa, live_intervals);
    vector_sort(live_intervals, ssa_compare_live_intervals);

    uint64_t* register_usage = (uint64_t*)mem_alloca(vector_size(live_intervals) * sizeof(uint64_t));
    memset(register_usage, 0, vector_size(live_intervals) * sizeof(uint64_t));

    for(uint32_t i = 0; i < vector_size(live_intervals); i++)
    {
        SSALiveInterval* current = (SSALiveInterval*)vector_at(live_intervals, i);

        if(current->reg != NO_REG)
        {
            for(uint32_t j = (current->start - 1); j < current->end; j++)
            {
                SET_BIT64(register_usage[j], current->reg);
            }
        }
    }

    while(true)
    {
        ConflictGroup conflict;
        bool any_conflict = false;

        for(uint32_t i = 0; i < vector_size(live_intervals); i++)
        {
            SSALiveInterval* current = (SSALiveInterval*)vector_at(live_intervals, i);
            
            if(current->reg != NO_REG)
            {
                for(uint32_t j = 0; j < i; j++)
                {
                    SSALiveInterval* other = (SSALiveInterval*)vector_at(live_intervals, j);

                    if(other->reg == current->reg && 
                       current->start < other->end && 
                       current->end > other->start)
                    {
                        uint32_t first, second;

                        if((current->end - current->start) <= (other->end - other->start)) 
                        {
                            conflict.first = i;
                            conflict.second = j;
                        } 
                        else 
                        {
                            conflict.first = j;
                            conflict.second = i;
                        }

                        any_conflict = true;

                        goto resolve_conflict;
                    }
                }
            }
        }

resolve_conflict:
        if(any_conflict)
        {
            SSALiveInterval* conflicting = (SSALiveInterval*)vector_at(live_intervals, conflict.second);

            uint32_t move_point = conflicting->start;
            uint32_t new_reg = ctz_u64(~register_usage[move_point]);

            if(new_reg < num_registers)
            {
                SSAInstruction* move = mathsexpr_ssa_new_move(ssa, conflicting->instruction, new_reg);
                SSALiveInterval move_interval = { move, new_reg, move_point + 1, conflicting->end };

                for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
                {
                    SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);
                    mathsexpr_ssa_replace_instructions_deps(ssa, instruction, conflicting->instruction, move);
                }

                conflicting->end = move_point + 1;

                for(uint32_t i = move_point; i < conflicting->end; i++)
                {
                    SET_BIT64(register_usage[i], new_reg);
                }

                vector_insert(ssa->instructions, &move, move_point);
                vector_insert(live_intervals, &move_interval, 0);
            }
            else
            {
                conflicting->reg = NO_REG;
                vector_free(live_intervals);
                logger_log_error("Register pressure too high in conflict resolution, need to spill");
                return false;
            }
        }
        else
        {
            break;
        }
    }

    ssa_find_last_uses(ssa, live_intervals);

    uint32_t stack_offset = 0;
    uint32_t stack_size = 0;

    for(uint32_t i = 0; i < vector_size(live_intervals); i++)
    {
        SSALiveInterval* current = (SSALiveInterval*)vector_at(live_intervals, i);

        if(current->reg != NO_REG)
        {
            mathsexpr_ssa_set_instruction_destination(current->instruction, current->reg);
            continue;
        }

        if(current->instruction->type == SSAInstructionType_SSABinOP)
        {
            SSABinOP* binop = SSA_CAST(SSABinOP, current->instruction);
            MATHSEXPR_ASSERT(binop != NULL, "Wrong type casting, should be SSABinOP");

            SSALiveInterval* left = ssa_get_operand_interval(ssa, binop->left, live_intervals);
            SSALiveInterval* right = ssa_get_operand_interval(ssa, binop->right, live_intervals);

            if(left && left->last_use <= current->start) 
            {
                current->reg = left->reg;
                mathsexpr_ssa_set_instruction_destination(current->instruction, current->reg);
                continue;
            }
            else if(right && right->last_use <= current->start) 
            {
                current->reg = right->reg;
                mathsexpr_ssa_set_instruction_destination(current->instruction, current->reg);
                continue;
            }
        }

        uint64_t used = 0;

        for(uint32_t j = (current->start - 1); j < current->end; j++)
        {
            used |= register_usage[j];
        }

        if(~used)
        {
            current->reg = ctz_u64(~used);

            for(uint32_t j = (current->start - 1); j < current->end; j++)
            {
                SET_BIT64(register_usage[j], current->reg);
            }
        }
        else
        {
            logger_log_error("Register pressure too high, need to spill");
            vector_free(live_intervals);
            return false;
        }

        mathsexpr_ssa_set_instruction_destination(current->instruction, current->reg);
    }

    vector_free(live_intervals);

    SET_FLAG(ssa->flags, SSAFlags_HasRegistersAsDestination);

    return true;
}