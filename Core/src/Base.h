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
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    Unique<T> CreateShared(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
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

#ifdef FOO_DEBUG
#if defined(_WIN32)
#define FOO_DEBUGBREAK() __debugbreak()
#endif
#define FOO_ENABLE_ASSERTS
#else
#define FOO_DEBUGBREAK()
#endif

#define FOO_EXPAND_MACRO(x)    x
#define FOO_STRINGIFY_MACRO(x) #x

#define CONCATANATE_MACROS(x, y)  x##y
#define CONCATANATE_MACROS2(x, y) CONCATANATE_MACROS(x, y)

#define DEFER(x) Defer CONCATANATE_MACROS2(defer, __LINE__)([&] { x; });
}  // namespace FooGame
#include "Util.h"
