#pragma once

#include <cstddef>
#include <cstdint>

using hash_t = uint64_t;

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

template <typename T>
struct DefaultHash
{
    size_t operator()(T x) { return Hash(x); }
};
