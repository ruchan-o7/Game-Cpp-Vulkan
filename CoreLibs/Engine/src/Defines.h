#pragma once
namespace Engine
{

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define ARRAY_COUNT(x)   (sizeof(x) / sizeof(x[0]))
#define ENGINE_NAMESPACE FooGame

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
#define BIT(x)           (1 << x)
#define VS_CODE_DEBUGGER 0

#define NOT_IMPLEMENTED() assert(0 && "Not implemented yet")

}  // namespace Engine
