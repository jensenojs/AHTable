#pragma once
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

using StringKey8HashTable = HashTable<HashTableCell<StringKey8>, DefaultAllocator, HashTableGrower<>>;
using StringKey16HashTable = HashTable<HashTableCell<StringKey16>, DefaultAllocator, HashTableGrower<>>;
using StringKey24HashTable = HashTable<HashTableCell<StringKey24>, DefaultAllocator, HashTableGrower<>>;
using StringRefHashTable = HashTable<HashTableCell<duckdb::string_t>, DefaultAllocator, HashTableGrower<>>;

template <typename MappedType>
struct StringHashTableResult
{
    duckdb::string_t key;
    MappedType * value;

    StringHashTableResult() = default;

    StringHashTableResult(duckdb::string_t key, MappedType * value) : key(std::move(key)), value(value) { }

    const duckdb::string_t & GetKey() const { return key; }
    MappedType & GetValue() const { return *value; }
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
                    memcpy(&sum_type_key.n3[0], key_ptr, 8);
                    sum_type_key.n3[0] &= (size_t)0xFFFFFFFFFFFFFFFF >> tail;
                }
                else
                {
                    memcpy(&sum_type_key.n3[0], key_ptr + key_size - 8, 8);
                    sum_type_key.n3[0] >>= tail;
                }
                StringKey8HashTable::result_type res;
                t1.Emplace(sum_type_key.k8, res);
                result.value = &res->GetValue();
                break;
            }
            case 1: {
                memcpy(&sum_type_key.n3[0], key_ptr, 8);
                memcpy(&sum_type_key.n3[1], key_ptr + key_size - 8, 8);
                sum_type_key.n3[1] >>= tail;
                StringKey16HashTable::result_type res;
                t2.Emplace(sum_type_key.k16, res);
                result.value = &res->GetValue();
                break;
            }
            case 2: {
                memcpy(&sum_type_key.n3[0], key_ptr, 16);
                memcpy(&sum_type_key.n3[2], key_ptr + key_size - 8, 8);
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

    void DispatchEmplace(duckdb::string_t key) { }

private:
    StringKey8HashTable t1;
    StringKey16HashTable t2;
    StringKey24HashTable t3;
    StringRefHashTable st;
};