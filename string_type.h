//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/types/string_type.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>
#include <cstdint>
#include <string>

#include <cstring>

namespace duckdb
{
struct string_t
{
public:
    static constexpr size_t PREFIX_LENGTH = 4 * sizeof(char);
    static constexpr size_t INLINE_LENGTH = 12;

    string_t() = default;
    explicit string_t(uint32_t len) { value.inlined.length = len; }
    string_t(const char * data, uint32_t len)
    {
        value.inlined.length = len;
        assert(data || GetSize() == 0);
        if (IsInlined())
        {
            // zero initialize the prefix first
            // this makes sure that strings with length smaller than 4 still have an equal prefix
            memset(value.inlined.inlined, 0, INLINE_LENGTH);
            if (GetSize() == 0)
            {
                return;
            }
            // small string: inlined
            memcpy(value.inlined.inlined, data, GetSize());
        }
        else
        {
            // large string: store pointer
            memcpy(value.pointer.prefix, data, PREFIX_LENGTH);
            value.pointer.ptr = (char *)data;
        }
    }
    string_t(const char * data) : string_t(data, strlen(data))
    { // NOLINT: Allow implicit conversion from `const char*`
    }
    string_t(const std::string & value) : string_t(value.c_str(), value.size())
    { // NOLINT: Allow implicit conversion from `const char*`
    }

    bool IsInlined() const { return GetSize() <= INLINE_LENGTH; }

    //! this is unsafe since the string will not be terminated at the end
    const char * GetDataUnsafe() const { return IsInlined() ? (const char *)value.inlined.inlined : value.pointer.ptr; }

    char * GetDataWriteable() const { return IsInlined() ? (char *)value.inlined.inlined : value.pointer.ptr; }

    const char * GetPrefix() const { return value.pointer.prefix; }

    size_t GetSize() const { return value.inlined.length; }

    std::string GetString() const { return std::string(GetDataUnsafe(), GetSize()); }

    explicit operator std::string() const { return GetString(); }

    void Finalize()
    {
        // set trailing NULL byte
        auto dataptr = (char *)GetDataUnsafe();
        if (GetSize() <= INLINE_LENGTH)
        {
            // fill prefix with zeros if the length is smaller than the prefix length
            for (size_t i = GetSize(); i < INLINE_LENGTH; i++)
            {
                value.inlined.inlined[i] = '\0';
            }
        }
        else
        {
            // copy the data into the prefix
            memcpy(value.pointer.prefix, dataptr, PREFIX_LENGTH);
        }
    }
    
    bool operator<(const string_t & r) const
    {
        auto this_str = this->GetString();
        auto r_str = r.GetString();
        return this_str < r_str;
    }

private:
    union
    {
        struct
        {
            uint32_t length;
            char prefix[4];
            char * ptr;
        } pointer;
        struct
        {
            uint32_t length;
            char inlined[12];
        } inlined;
    } value;
};

} // namespace duckdb
