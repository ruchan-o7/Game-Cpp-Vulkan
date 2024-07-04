#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace FooGame
{
    struct Camera
    {
            enum class Type
            {
                FirstPerson,
                LookAt
            };
            Type type = Type::FirstPerson;

            glm::vec3 Rotation = glm::vec3();
            glm::vec3 Position = glm::vec3();
            glm::vec3 Front    = glm::vec3();
            float Fov = 90, Aspect = 16.0f / 9.f, ZNear = 0.01f, ZFar = 1000.0f;

            glm::mat4 Perspective, View;
            bool FlipY = true;

            void UpdateMatrix();
    };

}  // namespace FooGame
