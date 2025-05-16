// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/log.h"

#include <stdio.h>

#if defined(MATHSEXPR_WIN)
#include <Windows.h>
#endif // defined(MATHSEXPR_WIN)

void MATHSEXPR_LIB_ENTRY lib_entry() noexcept
{
#if MATHSEXPR_DEBUG
    mathsexpr::log_debug("mathsexpr entry");
#endif // MATHSEXPR_DEBUG
}

void MATHSEXPR_LIB_EXIT lib_exit() noexcept
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