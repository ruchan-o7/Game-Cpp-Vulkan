#pragma once
#include <memory>
#include <utility>

#define u32 uint32_t
#define i32 int32_t

#define u16 uint16_t
#define i16 int16_t

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn)                    \
    [this](auto&&... args) -> decltype(auto) \
    { return this->fn(std::forward<decltype(args)>(args)...); }
#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

template <typename T>
using Shared = std::shared_ptr<T>;
template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr Shared<T> CreateShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
template <typename T, typename... Args>
constexpr Unique<T> CreateUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}
