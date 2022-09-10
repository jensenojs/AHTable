#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

struct DefaultAllocator
{
    void * Alloc(size_t size)
    {
        void * buf = malloc(size);
        if (buf == nullptr)
        {
            throw std::logic_error("Can't not allocate memoery in DefaultAllocator::Alloc()");
        }
        memset(buf, 0, size);
        return buf;
    }

    void Free(void * ptr) { free(ptr); }

    void * Realloc(void * ptr, size_t new_size) { return realloc(ptr, new_size); }
};
