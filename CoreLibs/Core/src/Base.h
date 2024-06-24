#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <utility>
#include <string>
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
#define DELETE_COPY(x)               \
public:                              \
    x(const x&)            = delete; \
    x& operator=(const x&) = delete;

#define DELETE_MOVE(x)          \
public:                         \
    x(x&&)            = delete; \
    x& operator=(x&&) = delete;

#define DELETE_COPY_MOVE(type) \
    DELETE_COPY(type)          \
    DELETE_MOVE(type)

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

#define u64 uint64_t
#define i64 int64_t

#define u32 uint32_t
#define i32 int32_t

#define i16 int16_t
#define i16 int16_t

#define i8 int8_t
#define u8 uint8_t

#define DEFAULT_MATERIAL_NAME "Default Material"
#define DEFAULT_TEXTURE_NAME  "Default Texture"

#define ENGINE_NAMESPACE FooGame

#define BIT(x) (1 << x)

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

#define BIND_EVENT_FN(fn)                    \
    [this](auto&&... args) -> decltype(auto) \
    { return this->fn(std::forward<decltype(args)>(args)...); }

#define NOT_IMPLEMENTED() assert(0 && "Not implemented yet")
#define DEPRECATED()                                                               \
    FOO_ENGINE_CRITICAL("This function deprecated: {0}, {1}", __FILE__, __LINE__); \
    assert(0)

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

}  // namespace FooGame
#include "Util.h"