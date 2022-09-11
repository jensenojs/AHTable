#pragma once
#include <type_traits>
#include "fast_memcpy.h"
#include "hash_table.h"
#include "string_heap.h"
#include "string_type.h"

using StringKey8 = uint64_t;

struct StringKey16
{
    uint64_t a;
    uint64_t b;

    bool operator==(const StringKey16 & rhs) const { return a == rhs.a && b == rhs.b; }
    bool operator!=(const StringKey16 & rhs) const { return !this->operator==(rhs); }
};

struct StringKey24
{
    uint64_t a;
    uint64_t b;
    uint64_t c;

    bool operator==(const StringKey24 & rhs) const { return a == rhs.a && b == rhs.b && c == rhs.c; }
    bool operator!=(const StringKey24 & rhs) const { return !this->operator==(rhs); }
};


inline duckdb::string_t ToDuckDBString(const StringKey8 & x)
{
    for (size_t i = 8; i > 0; i--)
    {
        if ((x & (0x1 << (i - 1))) != 0)
        {
            return duckdb::string_t((const char *)(&x), i);
        }
    }
    return duckdb::string_t();
}

inline duckdb::string_t ToDuckDBString(const StringKey16 & x)
{
    for (size_t i = 8; i > 0; i--)
    {
        if ((x.b & (0x1 << (i - 1))) != 0)
        {
            return duckdb::string_t((const char *)(&x), i + 8);
        }
    }
    return duckdb::string_t();
}

inline duckdb::string_t ToDuckDBString(const StringKey24 & x)
{
    for (size_t i = 8; i > 0; i--)
    {
        if ((x.c & (0x1 << (i - 1))) != 0)
        {
            return duckdb::string_t((const char *)(&x), i + 16);
        }
    }
    return duckdb::string_t();
}

template <>
hash_t Hash(StringKey16 x)
{
    hash_t a = Hash(x.a);
    hash_t b = Hash(x.b);
    return CombineHash(a, b);
}

template <>
hash_t Hash(StringKey24 x)
{
    hash_t res = Hash(x.a);
    res = CombineHash(res, Hash(x.b));
    res = CombineHash(res, Hash(x.c));
    return res;
}


/*
template <typename Cell>
struct StringHashTableResult
{
    Cell * cell = nullptr;

    StringHashTableResult() = default;

    StringHashTableResult(Cell * cell) : cell(cell) { }

    // const duckdb::string_t & GetKey() const { return cell->GetKey(); }
    // MappedType & GetValue() const { return cell->GetValue(); }

    explicit operator bool() const { return cell != nullptr; }

    Cell & operator*() const { return *cell; }

    Cell * operator->() const { return *cell; }

    friend bool operator==(const StringHashTableResult & lhs, const std::nullptr_t &) { return !lhs.cell; }
    friend bool operator==(const std::nullptr_t &, const StringHashTableResult & rhs) { return !rhs.cell; }
    friend bool operator!=(const StringHashTableResult & lhs, const std::nullptr_t &) { return lhs.cell; }
    friend bool operator!=(const std::nullptr_t &, const StringHashTableResult & rhs) { return rhs.cell; }
};
*/

template <typename T>
struct StringHashTableCell : public HashTableCell<T>
{
    using Base = HashTableCell<T>;
    using Base::Base;

    duckdb::string_t GetKey() const
    {
        const auto & key = Base::GetKey();
        return duckdb::string_t(key.data(), key.size());
    }

    const T & GetRawKey() const { return Base::GetRawKey(); }
    bool IsOccupied() const { return key.size() != 0; }
    void SetUnoccupied() { key.size() = 0; }
};

template <>
struct StringHashTableCell<StringKey8> : public HashTableCell<StringKey8>
{
    using Base = HashTableCell<StringKey8>;
    using Base::Base;

    // using key_type = duckdb::string_t;
    // using mapped_type = typename Base::mapped_type;

    duckdb::string_t GetKey() const { return ToDuckDBString(GetRawKey()); }
    const StringKey8 & GetRawKey() const { return Base::GetRawKey(); }
};

template <>
struct StringHashTableCell<StringKey16> : public HashTableCell<StringKey16>
{
    using Base = HashTableCell<StringKey16>;
    using Base::Base;

    // using key_type = duckdb::string_t;
    // using mapped_type = typename Base::mapped_type;

    duckdb::string_t GetKey() const { return ToDuckDBString(GetRawKey()); }
    const StringKey16 & GetRawKey() const { return Base::GetRawKey(); }
    bool IsOccupied() const { return key.a != 0; }
    void SetUnoccupied() { key.a = 0; }
};

template <>
struct StringHashTableCell<StringKey24> : public HashTableCell<StringKey24>
{
    using Base = HashTableCell<StringKey24>;
    using Base::Base;

    // using key_type = duckdb::string_t;
    // using mapped_type = typename Base::mapped_type;

    duckdb::string_t GetKey() const { return ToDuckDBString(GetRawKey()); }
    const StringKey24 GetRawKey() const { return Base::GetRawKey(); }
    bool IsOccupied() const { return key.a != 0; }
    void SetUnoccupied() { key.a = 0; }
};

template <>
struct StringHashTableCell<duckdb::string_t> : public HashTableCell<duckdb::string_t>
{
    using Base = HashTableCell<duckdb::string_t>;
    using Base::Base;

    using Base::GetRawKey;

    // using key_type = duckdb::string_t;
    // using mapped_type = typename Base::mapped_type;

    const duckdb::string_t & GetKey() const { return Base::GetKey(); }
    bool IsOccupied() const { return key.GetSize() != 0; }
    void SetUnoccupied() { key = duckdb::string_t(); }
};

using StringKey8HashTable = HashTable<StringHashTableCell<StringKey8>, DefaultAllocator, HTAssistant<>>;
using StringKey16HashTable = HashTable<StringHashTableCell<StringKey16>, DefaultAllocator, HTAssistant<>>;
using StringKey24HashTable = HashTable<StringHashTableCell<StringKey24>, DefaultAllocator, HTAssistant<>>;
using StringRefHashTable = HashTable<StringHashTableCell<duckdb::string_t>, DefaultAllocator, HTAssistant<>>;

using StdStringHashTable = HashTable<StringHashTableCell<std::string>, DefaultAllocator, HTAssistant<>>;

template <typename MappedType>
struct StringHashTableResult
{
    duckdb::string_t key;
    MappedType * value = nullptr;
    // Cell * cell = nullptr;

    StringHashTableResult() = default;

    explicit StringHashTableResult(duckdb::string_t key, MappedType * value) : key(key), value(value) { }

    const duckdb::string_t & GetKey() const { return key; }
    MappedType & GetValue() const { return *value; }

    explicit operator bool() const { return value != nullptr; }

    StringHashTableResult & operator*() const { return *this; }

    StringHashTableResult * operator->() const { return this; }

    // Cell & operator*() const { return *this; }

    // Cell * operator->() const { return this; }

    friend bool operator==(const StringHashTableResult & lhs, const std::nullptr_t &) { return !lhs.value; }
    friend bool operator==(const std::nullptr_t &, const StringHashTableResult & rhs) { return !rhs.value; }
    friend bool operator!=(const StringHashTableResult & lhs, const std::nullptr_t &) { return lhs.value; }
    friend bool operator!=(const std::nullptr_t &, const StringHashTableResult & rhs) { return rhs.value; }
};

template <>
struct StringHashTableResult<void>
{
    duckdb::string_t key;
    bool has_key = false;

    StringHashTableResult() = default;

    explicit StringHashTableResult(duckdb::string_t key) : key(key), has_key(true) { }

    const duckdb::string_t & GetKey() const { return key; }
    // void GetValue() const { return; }

    explicit operator bool() const { return has_key; }

    StringHashTableResult & operator*() { return *this; }

    StringHashTableResult * operator->() { return this; }

    // Cell & operator*() const { return *this; }

    // Cell * operator->() const { return this; }

    friend bool operator==(const StringHashTableResult & lhs, const std::nullptr_t &) { return !lhs.has_key; }
    friend bool operator==(const std::nullptr_t &, const StringHashTableResult & rhs) { return !rhs.has_key; }
    friend bool operator!=(const StringHashTableResult & lhs, const std::nullptr_t &) { return lhs.has_key; }
    friend bool operator!=(const std::nullptr_t &, const StringHashTableResult & rhs) { return rhs.has_key; }
};

//! SAHA: A String Adaptive Hash Table for Analytical Databases
//! https://www.mdpi.com/2076-3417/10/6/1915
struct StringHashTable
{
public:
    union StringKey
    {
        StringKey8 k8;
        StringKey16 k16;
        StringKey24 k24;
        uint64_t n3[3];
    };

    StringHashTable() = default;

public:
    using key_type = duckdb::string_t;
    using mapped_type = typename StringRefHashTable::mapped_type;
    using result_type = StringHashTableResult<typename StringRefHashTable::cell_type::mapped_type>;

    bool __attribute__((__always_inline__)) Emplace(duckdb::string_t key, result_type & result)
    {
        size_t key_size = key.GetSize();
        size_t tail = (~key_size + 1) << 3;
        size_t x = (key_size - 1) >> 3;
        StringKey sum_type_key;
        const char * key_ptr = key.GetDataUnsafe();
        switch (x)
        {
            case 0: {
                if (((uintptr_t)key_ptr & 0x800) == 0)
                {
                    FastMemcpy(&sum_type_key.n3[0], key_ptr, 8);
                    sum_type_key.n3[0] &= (size_t)0xFFFFFFFFFFFFFFFF >> tail;
                }
                else
                {
                    FastMemcpy(&sum_type_key.n3[0], key_ptr + key_size - 8, 8);
                    sum_type_key.n3[0] >>= tail;
                }
                StringKey8HashTable::result_type res;
                t1.Emplace(sum_type_key.k8, res);
                result.key = res->GetKey();
                if constexpr (!std::is_same<mapped_type, void>::value)
                {
                }
                break;
            }
            case 1: {
                FastMemcpy(&sum_type_key.n3[0], key_ptr, 8);
                FastMemcpy(&sum_type_key.n3[1], key_ptr + key_size - 8, 8);
                sum_type_key.n3[1] >>= tail;
                StringKey16HashTable::result_type res;
                t2.Emplace(sum_type_key.k16, res);
                result.key = res->GetKey();
                if constexpr (!std::is_same<mapped_type, void>::value)
                {
                }
                break;
            }
            case 2: {
                FastMemcpy(&sum_type_key.n3[0], key_ptr, 16);
                FastMemcpy(&sum_type_key.n3[2], key_ptr + key_size - 8, 8);
                sum_type_key.n3[2] >>= tail;
                StringKey24HashTable::result_type res;
                t3.Emplace(sum_type_key.k24, res);
                result.key = res->GetKey();
                if constexpr (!std::is_same<mapped_type, void>::value)
                {
                }
                break;
            }
            default:
                // TODO(lokax): Need to copy data
                duckdb::string_t new_key = heap.AddString(key);
                StringRefHashTable::result_type res;
                st.Emplace(new_key, res);
                result.key = res->GetKey();
                if constexpr (!std::is_same<mapped_type, void>::value)
                {
                }
                break;
        }
        return true;
    }

    result_type __attribute__((__always_inline__)) Lookup(const duckdb::string_t & key)
    {
        size_t key_size = key.GetSize();
        size_t tail = (~key_size + 1) << 3;
        size_t x = (key_size - 1) >> 3;
        StringKey sum_type_key;
        const char * key_ptr = key.GetDataUnsafe();
        switch (x)
        {
            case 0: {
                if (((uintptr_t)key_ptr & 0x800) == 0)
                {
                    FastMemcpy(&sum_type_key.n3[0], key_ptr, 8);
                    sum_type_key.n3[0] &= (size_t)0xFFFFFFFFFFFFFFFF >> tail;
                }
                else
                {
                    FastMemcpy(&sum_type_key.n3[0], key_ptr + key_size - 8, 8);
                    sum_type_key.n3[0] >>= tail;
                }
                auto res = t1.Lookup(sum_type_key.k8);
                if (!res)
                {
                    return result_type();
                }
                if constexpr (std::is_same<mapped_type, void>::value)
                {
                    return result_type(res->GetKey());
                }
                else
                {
                    // return result_type(res->GetKey(), &res->GetValue());
                }
            }
            case 1: {
                FastMemcpy(&sum_type_key.n3[0], key_ptr, 8);
                FastMemcpy(&sum_type_key.n3[1], key_ptr + key_size - 8, 8);
                sum_type_key.n3[1] >>= tail;
                auto res = t2.Lookup(sum_type_key.k16);
                if (!res)
                {
                    return result_type();
                }
                if constexpr (std::is_same<mapped_type, void>::value)
                {
                    return result_type(res->GetKey());
                }
                else
                {
                    // return result_type(res->GetKey(), &res->GetValue());
                }
            }
            case 2: {
                FastMemcpy(&sum_type_key.n3[0], key_ptr, 16);
                FastMemcpy(&sum_type_key.n3[2], key_ptr + key_size - 8, 8);
                sum_type_key.n3[2] >>= tail;
                auto res = t3.Lookup(sum_type_key.k24);
                if (!res)
                {
                    return result_type();
                }
                if constexpr (std::is_same<mapped_type, void>::value)
                {
                    return result_type(res->GetKey());
                }
                else
                {
                    // return result_type(res->GetKey(), &res->GetValue());
                }
            }
            default: {
                auto res = st.Lookup(key);
                if (!res)
                {
                    return result_type();
                }
                if constexpr (std::is_same<mapped_type, void>::value)
                {
                    return result_type(res->GetKey());
                }
                else
                {
                    // return result_type(res->GetKey(), &res->GetValue());
                }
            }
        }
    }

    size_t Size() const { return t1.Size() + t2.Size() + t3.Size() + st.Size(); }

    std::string PrintSize()
    {
        std::string res;
        res += "size: <t1> " + std::to_string(t1.Size()) + ", <t2> " + std::to_string(t2.Size()) + ", <t3> " + std::to_string(t3.Size())
            + ", <st> " + std::to_string(st.Size());
        return res;
    }

private:
    StringKey8HashTable t1;
    StringKey16HashTable t2;
    StringKey24HashTable t3;
    StringRefHashTable st;
    StringHeap heap;
};