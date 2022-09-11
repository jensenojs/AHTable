#pragma once

#include <cassert>
#include <memory>
#include <vector>
#include "string_type.h"

struct StringHeap
{
    duckdb::string_t AddString(const duckdb::string_t & s)
    {
        if (!chunk || remaining_size < s.GetSize())
        {
            auto memory = std::unique_ptr<char[]>(new char[4096]);
            chunk = memory.get();
            chunks.push_back(std::move(memory));
            remaining_size = 4096;
        }
        assert(remaining_size >= s.GetSize());
        memcpy(chunk, s.GetDataUnsafe(), s.GetSize());
        auto res = chunk;
        chunk += s.GetSize();
        return duckdb::string_t(res, s.GetSize());
    }

    char * chunk = nullptr;
    size_t remaining_size = 0;
    std::vector<std::unique_ptr<char[]>> chunks;
};