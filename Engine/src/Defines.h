#pragma once
// #include <cstdint>
// #include <string>
// #include <memory>
// #include <unordered_map>
// #include <vector>
namespace Engine
{

//     typedef uint32_t u32;
//     typedef int32_t i32;
//
//     typedef uint64_t u64;
//     typedef int64_t i64;
//
//     typedef uint16_t u16;
//     typedef int16_t i16;
//
//     using String = std::string;
//
//     template <typename T>
//     using List = std::vector<T>;
//
//     template <typename T>
//     using Shared = std::shared_ptr<T>;
//
//     template <typename T>
//     using Unique = std::unique_ptr<T>;
//     template <typename K, typename V>
//     using HashMap = std::unordered_map<K, V>;
//
#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))
//
#define DELETE_COPY(x)               \
public:                              \
    x(const x&)            = delete; \
    x& operator=(const x&) = delete;
#define BIT(x) (1 << x)
}  // namespace Engine
