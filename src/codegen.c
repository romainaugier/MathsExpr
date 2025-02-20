// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

/* 
    My best friends at the moment: 
     - https://www.felixcloutier.com/x86/ 
     - https://defuse.ca/online-x86-assembler.htm
     - https://godbolt.org/
*/

#include "mathsexpr/codegen.h"

const char* codegen_get_scalar_register_name(uint16_t reg)
{
    switch(reg)
    {
        case 0:
            return "xmm0";
        case 1:
            return "xmm1";
        case 2:
            return "xmm2";
        case 3:
            return "xmm3";
        case 4:
            return "xmm4";
        case 5:
            return "xmm5";
        case 6:
            return "xmm6";
        case 7:
            return "xmm7";
    }
}

const char* codegen_get_scalar_func_prefix()
{
    return "__s_";
}

void ssa_instruction_to_text_scalar(SSAInstruction* instruction, String* out)
{
    switch(instruction->type)
    {
        case SSAInstructionType_SSABinOP:
        {
            SSABinOP* binop = SSA_CAST(SSABinOP, instruction);
            MATHSEXPR_ASSERT(binop != NULL, "Wrong type casting, should be SSABinOP");
            
            char* text_instruction = "";

            switch(binop->op)
            {
                case SSABinOPType_Add:
                {
                    text_instruction = "addss";
                    break;
                }

                case SSABinOPType_Sub:
                {
                    text_instruction = "subss";
                    break;
                }

                case SSABinOPType_Mul:
                {
                    text_instruction = "mulss";
                    break;
                }

                case SSABinOPType_Div:
                {
                    text_instruction = "divss";
                    break;
                }

                case SSABinOPType_Pow:
                {
                    text_instruction = "addss";
                    break;
                }

                case SSABinOPType_Mod:
                {
                    text_instruction = "addss";
                    break;
                }
            }

            uint16_t regleft = mathsexpr_ssa_get_instruction_destination(binop->left);
            uint16_t regright = mathsexpr_ssa_get_instruction_destination(binop->right);
            uint16_t regout = binop->destination;

            if(binop->left->type == SSAInstructionType_SSALiteral)
            {
                SSALiteral* lit = SSA_CAST(SSALiteral, binop->left);
                string_appendf(out, "movss %s, %.3f\n",
                               codegen_get_scalar_register_name(regleft),
                               lit->value);
            }

            if(binop->right->type == SSAInstructionType_SSALiteral)
            {
                SSALiteral* lit = SSA_CAST(SSALiteral, binop->right);
                string_appendf(out, "movss %s, %.3f\n",
                               codegen_get_scalar_register_name(regright),
                               lit->value);
            }

            string_appendf(out, "%s %s, %s\n", 
                           text_instruction, 
                           codegen_get_scalar_register_name(regleft),
                           codegen_get_scalar_register_name(regright));

            if(regout != regleft)
            {
                string_appendf(out, "movss %s, %s\n", 
                               codegen_get_scalar_register_name(regout),
                               codegen_get_scalar_register_name(regleft));
            }

            break;
        }

        case SSAInstructionType_SSAFunction:
        {
            SSAFunction* func = SSA_CAST(SSAFunction, instruction);

            string_appendf(out, "call %s%s\n", 
                           codegen_get_scalar_func_prefix(),
                           mathsexpr_ssa_func_type_as_string(func->func));
            break;
        }
    }
}

const char* ssa_instruction_to_text_vector4(SSAInstruction* instruction)
{
    switch(instruction->type)
    {
        case SSAInstructionType_SSABinOP:
        {

        }
        default:
            return "";
    }
}

const char* ssa_instruction_to_text_vector8(SSAInstruction* instruction)
{
    switch(instruction->type)
    {
        case SSAInstructionType_SSABinOP:
        {

        }
        default:
            return "";
    }
}

void ssa_instruction_to_text(SSAInstruction* instruction, String* output)
{
    switch(instruction->type)
    {
        case SSAInstructionType_SSABinOP:
            break;
        default:
            return "";
    }
}

void mathsexpr_codegen_to_text(SSA* ssa, String* output, uint64_t flags)
{
    for(uint32_t i = 0; i < mathsexpr_ssa_num_instructions(ssa); i++)
    {
        SSAInstruction* instruction = mathsexpr_ssa_instruction_at(ssa, i);

        ssa_instruction_to_text_scalar(instruction, output);
    }

    string_appendc(output, "ret\n");
}

void mathsexpr_codegen_to_bytecode(SSA* ssa, Arena* output, uint64_t flags)
{

}