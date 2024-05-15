#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
namespace FooGame
{
#define DEFAULT(X)         \
public:                    \
    X()         = default; \
    X(const X&) = default;

    struct TransformComponent
    {
            glm::vec3 Position{0.0f, 0.0f, 0.0f};
            glm::vec3 Rotation{0.0f, 0.0f, 0.0f};
            glm::vec3 Scale{1.0f, 1.0f, 1.0f};
            DEFAULT(TransformComponent)
            TransformComponent(const glm::vec3& pos) : Position(pos) {}
            glm::mat4 GetTransform() const
            {
                glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
                return glm::translate(glm::mat4(1.0f), Position) * rotation *
                       glm::scale(glm::mat4(1.0f), Scale);
            }
    };
}  // namespace FooGame
