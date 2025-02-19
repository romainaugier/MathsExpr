// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/codegen.h"

const char* codegen_instruction_type_to_str(InstructionType type)
{
    switch(type)
    {
        case InstructionType_InstructionLoad:
            return "load";
        case InstructionType_InstructionStore:
            return "store";
        default:
            return "unknown";
    }
}