#pragma once
#include "hash_table.h"
#include "string_type.h"

using StringKey8 = uint64_t;
using StringKey16 = uint64_t[2];
using StringKey24 = uint64_t[3];

using StringKey8HashTable = HashTable<HashTableCell<StringKey8>, DefaultAllocator, HashTableGrower<>>;
using StringKey16HashTable = HashTable<HashTableCell<StringKey16>, DefaultAllocator, HashTableGrower<>>;
using StringKey24HashTable = HashTable<HashTableCell<StringKey24>, DefaultAllocator, HashTableGrower<>>;

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
    
    void DispatchEmplace(duckdb::string_t key) {

    }

private:
    StringKey8HashTable t1;
    StringKey16HashTable t2;
    StringKey24HashTable t3;
};