#pragma once
#include <glm/glm.hpp>

namespace FooGame
{

    struct UniformBufferObject
    {
            alignas(16) glm::mat4 View;
            alignas(16) glm::mat4 Projection;
            alignas(16) glm::mat4 Reserved0;
            alignas(16) glm::mat4 Reserved1;
    };
}  // namespace FooGame
