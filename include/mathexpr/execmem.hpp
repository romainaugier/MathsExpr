// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_EXECMEM)
#define __MATHEXPR_EXECMEM

#include "mathexpr/bytecode.hpp"
#include "mathexpr/log.hpp"

#include <cstring>

#if defined(MATHEXPR_WIN)
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif /* defined(MATHEXPR_WIN) */

MATHEXPR_NAMESPACE_BEGIN

class MATHEXPR_API ExecMem 
{
private:

    void* _memory;
    size_t _size;
    bool _locked;

    bool allocate() noexcept
    {
#if defined(MATHEXPR_WIN)
        this->_memory = VirtualAlloc(nullptr, _size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        if(this->_memory == nullptr) 
        {
            log_error("Failed to allocate executable memory");
            return false;
        }
#else
        this->_memory = mmap(nullptr, _size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if(this->_memory == MAP_FAILED) 
        {
            log_error("Failed to allocate executable memory");
            return false;
        }
#endif /* defined(MATHEXPR_WIN) */

        return true;
    }

    void deallocate() 
    {
        if(this->_memory == nullptr) 
        {
            return;
        }

#if defined(MATHEXPR_WIN)
        VirtualFree(this->_memory, 0, MEM_RELEASE);
#else
        munmap(this->_memory, this->_size);
#endif /* defined(MATHEXPR_WIN) */
        this->_memory = nullptr;
    }
public:
    using FunctionType = double(*)(const double*, const double*);

    ExecMem() : _memory(nullptr), _size(0), _locked(false) {}

    ExecMem(size_t size) : _memory(nullptr), _size(size), _locked(false) 
    {
        this->allocate();
    }

    ~ExecMem() 
    {
        this->deallocate();
    }

    ExecMem(const ExecMem&) = delete;
    ExecMem& operator=(const ExecMem&) = delete;

    ExecMem(ExecMem&& other) noexcept : _memory(other._memory), 
                                        _size(other._size), 
                                        _locked(other._locked) 
    {
        other._memory = nullptr;
        other._size = 0;
        other._locked = false;
    }

    ExecMem& operator=(ExecMem&& other) noexcept 
    {
        if(this != &other) 
        {
            this->deallocate();
            this->_memory = other._memory;
            this->_size = other._size;
            this->_locked = other._locked;
            other._memory = nullptr;
            other._size = 0;
            other._locked = false;
        }

        return *this;
    }

    bool write(const ByteCode& bytecode) noexcept
    {
        if(this->_locked) 
        {
            log_error("Cannot write to locked memory");
            return false;
        }

        if(bytecode.size() > this->_size) 
        {
            log_error("Cannot write to locked memory");
            return false;
        }

        std::memcpy(this->_memory, bytecode.data(), bytecode.size());

        return true;
    }

    bool lock() noexcept
    {
        if(this->_locked) 
        {
            return true;
        }
        
#if defined(MATHEXPR_WIN)
        DWORD oldProtect;

        if(!VirtualProtect(this->_memory, this->_size, PAGE_EXECUTE_READ, &oldProtect)) 
        {
            log_error("Failed to make memory executable");
            return false;
        }
#else
        if(mprotect(this->_memory, this->_size, PROT_READ | PROT_EXEC) != 0) 
        {
            log_error("Failed to make memory executable");
            return false;
        }
#endif /* defined(MATHEXPR_WIN) */
        this->_locked = true;

        return true;
    }

    FunctionType as_function() const noexcept
    {
        if(!this->_locked) 
        {
            log_error("ExecMem must be locked before casting to function");
            return nullptr;
        }

        return reinterpret_cast<FunctionType>(_memory);
    }

    size_t size() const { return this->_size; }
    bool is_locked() const { return this->_locked; }
};

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_EXECMEM) */