#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

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

namespace details
{
    template<class>
    struct hasher;
    template<>
    struct hasher<std::string>
    {
        uint64_t constexpr operator()(char const* input) const
        {
            return *input ? static_cast<unsigned int>(*input) + 33 * (*this)(input + 1) : 5381;
        }
        uint64_t operator()(const std::string& str) const { return (*this)(str.c_str()); }
    };

    template<>
    struct hasher<const char*>
    {
        uint64_t constexpr operator()(char const* input) const
        {
            return *input ? static_cast<unsigned int>(*input) + 33 * (*this)(input + 1) : 5381;
        }
        uint64_t operator()(const std::string& str) const { return (*this)(str.c_str()); }
    };

    constexpr int16_t  i16(const char* s, int16_t v) { return *s ? i16(s + 1, v * 256 + *s) : v; }
    constexpr int32_t  i32(const char* s, int32_t v) { return *s ? i32(s + 1, v * 256 + *s) : v; }
    constexpr int64_t  i64(const char* s, int64_t v) { return *s ? i64(s + 1, v * 256 + *s) : v; }
    constexpr uint16_t u16(const char* s, uint16_t v) { return *s ? u16(s + 1, v * 256 + *s) : v; }
    constexpr uint32_t u32(const char* s, uint32_t v) { return *s ? u32(s + 1, v * 256 + *s) : v; }
    constexpr uint64_t u64(const char* s, uint64_t v) { return *s ? u64(s + 1, v * 256 + *s) : v; }
} // namespace details

constexpr int16_t  operator"" _i16(const char* s, size_t) { return details::i16(s, 0); }
constexpr int32_t  operator"" _i32(const char* s, size_t) { return details::i32(s, 0); }
constexpr int64_t  operator"" _i64(const char* s, size_t) { return details::i64(s, 0); }
constexpr uint16_t operator"" _u16(const char* s, size_t) { return details::u16(s, 0); }
constexpr uint32_t operator"" _u32(const char* s, size_t) { return details::u32(s, 0); }
constexpr uint64_t operator"" _u64(const char* s, size_t) { return details::u64(s, 0); }

static inline uint64_t i16(const std::string& s) { return details::i16(s.c_str(), 0); }
static inline uint64_t i32(const std::string& s) { return details::i32(s.c_str(), 0); }
static inline uint64_t i64(const std::string& s) { return details::i64(s.c_str(), 0); }
static inline uint64_t u16(const std::string& s) { return details::u16(s.c_str(), 0); }
static inline uint64_t u32(const std::string& s) { return details::u32(s.c_str(), 0); }
static inline uint64_t u64(const std::string& s) { return details::u64(s.c_str(), 0); }

template<typename T>
constexpr uint64_t hash(T&& t)
{
    return ::details::hasher<typename std::decay<T>::type>()(std::forward<T>(t));
}
constexpr uint64_t operator"" _hash(const char* s, size_t) { return ::details::hasher<std::string>()(s); }
