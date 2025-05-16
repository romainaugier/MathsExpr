// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/log.h"

#include <stdio.h>

#if defined(MATHSEXPR_WIN)
#include <Windows.h>
#endif // defined(MATHSEXPR_WIN)

/* 
   In this source file we execute all functions that need to be executed at runtime to check and
   set some global variables (for simd vectorization, cpu frequency for profiling...) 

   lib_entry is executed on dlopen / LoadLibrary
   lib_exit is executed on dlclose / CloseLibrary
*/

void MATHSEXPR_LIB_ENTRY lib_entry(void)
{
#if MATHSEXPR_DEBUG
    mathsexpr::log_debug("mathsexpr entry");
#endif // MATHSEXPR_DEBUG
}

void MATHSEXPR_LIB_EXIT lib_exit(void)
{
#if MATHSEXPR_DEBUG
    mathsexpr::log_debug("mathsexpr exit");
#endif // MATHSEXPR_DEBUG
}

#if defined(MATHSEXPR_WIN)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) 
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            /* Code to run when the DLL is loaded */
            lib_entry();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            /* Code to run when the DLL is unloaded */
            lib_exit();
            break;
    }

    return TRUE;
}
#endif // defined(MATHSEXPR_WIN)