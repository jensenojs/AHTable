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