#pragma once
namespace Engine
{

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))
#define DELETE_COPY(x)               \
public:                              \
    x(const x&)            = delete; \
    x& operator=(const x&) = delete;
#define BIT(x)           (1 << x)
#define VS_CODE_DEBUGGER 1
}  // namespace Engine
