#pragma once
#include <Log.h>
#include "../Base.h"
#include <filesystem>

#ifdef FOO_ENABLE_ASSERTS

#define FOO_INTERNAL_ASSERT_IMPL(type, check, msg, ...) \
    {                                                   \
        if (!(check))                                   \
        {                                               \
            FOO_CORE##type##ERROR(msg, __VA_ARGS__);    \
            FOO_DEBUGBREAK();                           \
        }                                               \
    }
#define FOO_INTERNAL_ASSERT_WITH_MSG(type, check, ...) \
    FOO_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define FOO_INTERNAL_ASSERT_NO_MSG(type, check)                                \
    FOO_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", \
                             FOO_STRINGIFY_MACRO(check),                       \
                             std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define FOO_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define FOO_INTERNAL_ASSERT_GET_MACRO(...)                                                         \
    FOO_EXPAND_MACRO(FOO_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, FOO_INTERNAL_ASSERT_WITH_MSG, \
                                                        FOO_INTERNAL_ASSERT_NO_MSG))

#define FOO_ASSERT(...) FOO_EXPAND_MACRO(FOO_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__))
#define FOO_CORE_ASSERT(...) \
    FOO_EXPAND_MACRO(FOO_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__))
#else
#define FOO_ASSERT(...)
#define FOO_CORE_ASSERT(...)
#endif
