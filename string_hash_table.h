#pragma once
#include "fast_memcpy.h"
#include "hash_table.h"
#include "string_type.h"

using StringKey8 = uint64_t;

struct StringKey16
{
    uint64_t a;
    uint64_t b;
};

struct StringKey24
{
    uint64_t a;
    uint64_t b;
    uint64_t c;
};

using StringKey8HashTable = HashTable<HashTableCell<StringKey8>, DefaultAllocator, HTAssistant<>>;
using StringKey16HashTable = HashTable<HashTableCell<StringKey16>, DefaultAllocator, HTAssistant<>>;
using StringKey24HashTable = HashTable<HashTableCell<StringKey24>, DefaultAllocator, HTAssistant<>>;
using StringRefHashTable = HashTable<HashTableCell<duckdb::string_t>, DefaultAllocator, HTAssistant<>>;

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

    // TODO:
    const duckdb::string_t GetKey() const { return duckdb::string_t(); }

    bool IsOccupied() const { return key.b != 0; }
    void SetUnoccupied() { key.b == 0; }
};

template <>
struct StringHashTableCell<StringKey16> : public HashTableCell<StringKey16>
{
};

template <>
struct StringHashTableCell<StringKey24>
{
};

template <>
struct StringHashTableCell<duckdb::string_t>
{
};

template <typename MappedType>
struct StringHashTableResult
{
    MappedType * value = nullptr;

    StringHashTableResult() = default;

    explicit StringHashTableResult(MappedType * value) : value(value) { }

    None GetKey() const { return None{}; }
    MappedType & GetValue() const { return *value; }

    explicit operator bool() const { return value != nullptr; }

    StringHashTableResult & operator*() const { return *this; }

    StringHashTableResult * operator->() const { return *this; }

    friend bool operator==(const StringHashTableResult & lhs, const std::nullptr_t &) { return !lhs.value; }
    friend bool operator==(const std::nullptr_t &, const StringHashTableResult & rhs) { return !rhs.value; }
    friend bool operator!=(const StringHashTableResult & lhs, const std::nullptr_t &) { return lhs.value; }
    friend bool operator!=(const std::nullptr_t &, const StringHashTableResult & rhs) { return rhs.value; }
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
    using mapped_type = typename StringRefHashTable::cell_type::mapped_type;
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
                result.value = &res->GetValue();
                break;
            }
            case 1: {
                FastMemcpy(&sum_type_key.n3[0], key_ptr, 8);
                FastMemcpy(&sum_type_key.n3[1], key_ptr + key_size - 8, 8);
                sum_type_key.n3[1] >>= tail;
                StringKey16HashTable::result_type res;
                t2.Emplace(sum_type_key.k16, res);
                result.value = &res->GetValue();
                break;
            }
            case 2: {
                FastMemcpy(&sum_type_key.n3[0], key_ptr, 16);
                FastMemcpy(&sum_type_key.n3[2], key_ptr + key_size - 8, 8);
                sum_type_key.n3[2] >>= tail;
                StringKey24HashTable::result_type res;
                t3.Emplace(sum_type_key.k24, res);
                result.value = &res->GetValue();
                break;
            }
            default:

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
                return result_type(&res->GetValue());
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
                return result_type(&res->GetValue());
            }
            case 2: {
                FastMemcpy(&sum_type_key.n3[0], key_ptr, 16);
                FastMemcpy(&sum_type_key.n3[2], key_ptr + key_size - 8, 8);
                sum_type_key.n3[2] >>= tail;
                StringKey24HashTable::result_type res;
                auto res = t3.Lookup(sum_type_key.k24);
                if (!res)
                {
                    return result_type();
                }
                return result_type(&res->GetValue());
            }
            default: {
                // TODO(lokax): Need to copy data
                auto res = st.Lookup(key);
                if (!res)
                {
                    return result_type();
                }
                return result_type(&res->GetValue());
            }
        }
    }

    size_t Size() const { return t1.Size() + t2.Size() + t3.Size() + st.Size(); }

private:
    StringKey8HashTable t1;
    StringKey16HashTable t2;
    StringKey24HashTable t3;
    StringRefHashTable st;
};