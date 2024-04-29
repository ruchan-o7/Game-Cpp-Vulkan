#pragma once
#include <cstdint>

#define u32 uint32_t
#define i32 int32_t

#define u16 uint16_t
#define i16 int16_t

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn)                    \
    [this](auto&&... args) -> decltype(auto) \
    { return this->fn(std::forward<decltype(args)>(args)...); }
