#pragma once

#include <cstddef>
#include <functional>

template<typename T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    seed ^= std::hash<T> {}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename T, typename... Ts>
inline void hash_combine(std::size_t& seed, const T& v, Ts... rest)
{
    hash_combine(seed, v);
    if constexpr (sizeof...(Ts) > 1)
    {
        hash_combine(seed, rest...);
    }
}
