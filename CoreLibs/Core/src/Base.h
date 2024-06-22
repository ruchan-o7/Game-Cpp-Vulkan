#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <utility>
#include <string>
#include "Defer.h"
namespace FooGame
{
    template <typename T>
    using Shared = std::shared_ptr<T>;
    template <typename T>
    using Unique = std::unique_ptr<T>;

    using String = std::string;

    template <typename T>
    using List = std::vector<T>;

    template <typename Key, typename Value>
    using Hashmap = std::unordered_map<Key, Value>;

    template <typename T, typename... Args>
    Unique<T> CreateUnique(Args&&... args)
    {
        return std::make_unique<T>(std::forward(args...));
    }

    template <typename T, typename... Args>
    Unique<T> CreateShared(Args&&... args)
    {
        return std::make_shared<T>(std::forward(args...));
    }
#ifndef DELETE_COPY
#define DELETE_COPY(x)               \
public:                              \
    x(const x&)            = delete; \
    x& operator=(const x&) = delete;
#endif
#ifndef DELETE_MOVE
#define DELETE_MOVE(x)          \
public:                         \
    x(x&&)            = delete; \
    x& operator=(x&&) = delete;
#endif
#define DEFER(x)   \
    Defer defer_   \
    {              \
        [&] { x; } \
    }
#define DEFER_(F, C) \
    Defer defer_##C  \
    {                \
        [&] { F; }   \
    }
#ifndef ARRAY_COUNT
#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))
#endif

#define u64 uint64_t
#define i64 int64_t

#define u32 uint32_t
#define i32 int32_t

#define i16 int16_t
#define i16 int16_t

#define i8 int8_t
#define u8 uint8_t

}  // namespace FooGame