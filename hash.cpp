#include "hash.h"
#include <functional>

template <>
hash_t Hash(float x)
{
    return (hash_t)std::hash<float>()(x);
}

template <>
hash_t Hash(double x)
{
    return (hash_t)std::hash<double>()(x);
}

hash_t CombineHash(hash_t a, hash_t b)
{
    return a ^ b;
}

static hash_t HashBytes(void * ptr, size_t len) noexcept;

template <>
hash_t Hash(duckdb::string_t x)
{
    return HashBytes((void *)x.GetDataUnsafe(), x.GetSize());
}

template <>
hash_t Hash(std::string & x)
{
    return HashBytes((void *)x.data(), x.size());
}


template <typename T>
const T Load(const char * ptr)
{
    T ret;
    memcpy(&ret, ptr, sizeof(ret));
    return ret;
}

// MIT License
// Copyright (c) 2018-2021 Martin Ankerl
// https://github.com/martinus/robin-hood-hashing/blob/3.11.5/LICENSE
static hash_t HashBytes(void * ptr, size_t len) noexcept
{
    static constexpr uint64_t M = UINT64_C(0xc6a4a7935bd1e995);
    static constexpr uint64_t SEED = UINT64_C(0xe17a1465);
    static constexpr unsigned int R = 47;

    auto const * const data64 = static_cast<uint64_t const *>(ptr);
    uint64_t h = SEED ^ (len * M);

    size_t const n_blocks = len / 8;
    for (size_t i = 0; i < n_blocks; ++i)
    {
        auto k = Load<uint64_t>(reinterpret_cast<const char *>(data64 + i));

        k *= M;
        k ^= k >> R;
        k *= M;

        h ^= k;
        h *= M;
    }

    auto const * const data8 = reinterpret_cast<uint8_t const *>(data64 + n_blocks);
    switch (len & 7U)
    {
        case 7:
            h ^= static_cast<uint64_t>(data8[6]) << 48U;
        case 6:
            h ^= static_cast<uint64_t>(data8[5]) << 40U;
        case 5:
            h ^= static_cast<uint64_t>(data8[4]) << 32U;
        case 4:
            h ^= static_cast<uint64_t>(data8[3]) << 24U;
        case 3:
            h ^= static_cast<uint64_t>(data8[2]) << 16U;
        case 2:
            h ^= static_cast<uint64_t>(data8[1]) << 8U;
        case 1:
            h ^= static_cast<uint64_t>(data8[0]);
            h *= M;
        default:
            break;
    }
    h ^= h >> R;
    h *= M;
    h ^= h >> R;
    return static_cast<hash_t>(h);
}