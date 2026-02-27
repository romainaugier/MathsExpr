// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathexpr/log.hpp"

#include <stdio.h>

#if defined(MATHEXPR_WIN)
#include <Windows.h>
#endif // defined(MATHEXPR_WIN)

void MATHEXPR_LIB_ENTRY lib_entry() noexcept
{
#if MATHEXPR_DEBUG
    mathexpr::log_debug("mathexpr entry");
#endif // MATHEXPR_DEBUG
}

void MATHEXPR_LIB_EXIT lib_exit() noexcept
{
#if MATHEXPR_DEBUG
    mathexpr::log_debug("mathexpr exit");
#endif // MATHEXPR_DEBUG
}

#if defined(MATHEXPR_WIN)
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
#endif // defined(MATHEXPR_WIN)