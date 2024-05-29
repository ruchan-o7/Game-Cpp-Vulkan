#pragma once
#include "../Defines.h"
#include <glm/glm.hpp>
namespace FooGame
{
    class PerspectiveCamera
    {
        public:
            PerspectiveCamera();
            const glm::vec3 GetPosition() const { return m_Position; }
            const glm::mat4& GetView() const { return m_View; }

            const glm::mat4& GetProjection() const { return m_Projection; }
            void SetProjection(const glm::mat4& projection);
            void SetPosition(const glm::vec3& newPos);
            void SetAspect(float aspect);
            void Zoom(float amount = 10);
            void Pan(glm::vec2 delta);
            void Rotate(glm::vec2 delta);
            void RecalculateViewMatrix();

            float m_Zoom     = 90.0f;
            float m_Yaw      = 0.0f;
            float m_Pitch    = -90.0f;
            float m_NearClip = 0.01f;
            float m_FarClip  = 1000.0f;
            float m_Aspect   = 16.0f / 9.0f;

            glm::vec3 m_Up                   = {0.0f, 0.0f, 1.0f};
            glm::vec3 m_WorldUp              = {0.0f, 0.0f, 1.0f};
            glm::vec3 m_Right                = {0.0f, 1.0f, 0.0f};
            glm::vec3 m_Direction            = {0.0f, 1.0f, 0.0f};
            glm::vec2 m_InitialMousePosition = {0.0f, 0.0f};

        private:
            glm::vec3 GetUpDirection() const;
            glm::quat GetOrientation() const;
            glm::mat4 m_Projection = glm::mat4(1.0f);
            glm::mat4 m_View       = glm::mat4(1.0f);
            glm::vec3 m_Position   = glm::vec3{0.0f, 0.0f, 0.0f};
    };

}  // namespace FooGame
