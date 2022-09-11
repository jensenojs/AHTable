#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include "string_type.h"

using hash_t = uint64_t;

struct StringKey16;
struct StringKey24;

inline hash_t murmurhash64(uint64_t x)
{
    x ^= x >> 32;
    x *= 0xd6e8feb86659fd93U;
    x ^= x >> 32;
    x *= 0xd6e8feb86659fd93U;
    x ^= x >> 32;
    return x;
}

template <typename T>
inline hash_t Hash(T x)
{
    return murmurhash64((uint64_t)x);
}


template <>
hash_t Hash(float x);

template <>
hash_t Hash(double x);

template <>
hash_t Hash(StringKey16 x);

template <>
hash_t Hash(StringKey24 x);

template <>
hash_t Hash(duckdb::string_t x);

template <>
hash_t Hash(std::string & x);

template <typename T>
struct DefaultHash
{
    size_t operator()(T x) { return Hash(x); }
};

hash_t CombineHash(hash_t a, hash_t b);
