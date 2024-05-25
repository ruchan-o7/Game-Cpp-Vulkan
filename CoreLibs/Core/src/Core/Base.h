#pragma once
#include <memory>
#include <cstdio>
#include "../../pch.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define u32 uint32_t
#define i32 int32_t

#define u64 uint64_t
#define i64 int64_t

#define u16 uint16_t
#define i16 int16_t

template <typename T>
using List = std::vector<T>;

template <typename T>
using Shared = std::shared_ptr<T>;
template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr Unique<T> CreateUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
constexpr Shared<T> CreateShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn)                    \
    [this](auto&&... args) -> decltype(auto) \
    { return this->fn(std::forward<decltype(args)>(args)...); }

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))
using String = std::string;

template <typename... Args>
String StrFormat(const String& format, Args... args)
{
    i32 size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
    if (size_s <= 0)
    {
        std::cerr << "[ERROR] | "
                  << " Error during string format: " << format << std::endl;
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf{new char[size]};
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return String(buf.get(), buf.get() + size - 1);
}
#define DELETE_COPY(x)               \
public:                              \
    x(const x&)            = delete; \
    x& operator=(const x&) = delete;
