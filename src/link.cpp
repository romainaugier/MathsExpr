// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/link.hpp"
#include "mathexpr/libmaths.hpp"
#include "mathexpr/log.hpp"

MATHEXPR_NAMESPACE_BEGIN

bool relocate(ByteCode& bytecode, const Relocations& relocations) noexcept
{
    for(const auto& relocation : relocations)
    {
        if(relocation.reloc_type != RelocType_Abs64)
        {
            log_error("Relocation type other than Abs64 are not supported for now");
            return false;
        }

        const auto entry = libmaths::get_function_entry(relocation.symbol_name);

        if(entry == nullptr)
        {
            log_error("Cannot find symbol \"{}\"", relocation.symbol_name);
            return false;
        }

        /* TODO: modify when using simd to get the right ptr */
        uint64_t addr = reinterpret_cast<uint64_t>(entry->scalar_ptr);

        log_debug("Relocating symbol: \"{}\" (0x{:016x})", relocation.symbol_name, addr);

        for(int32_t i = 0; i < 8; i++)
            bytecode[relocation.bytecode_offset + i] = BYTE(addr >> (i * 8));
    }

    return true;
}

MATHEXPR_NAMESPACE_END
