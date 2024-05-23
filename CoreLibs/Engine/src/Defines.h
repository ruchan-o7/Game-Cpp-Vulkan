#pragma once
namespace Engine
{

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))
#define DELETE_COPY(x)               \
public:                              \
    x(const x&)            = delete; \
    x& operator=(const x&) = delete;
#define BIT(x)           (1 << x)
#define VS_CODE_DEBUGGER 0
}  // namespace Engine
