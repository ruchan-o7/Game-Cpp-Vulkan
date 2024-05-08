#pragma once
#include "Base.h"
#include "glm/fwd.hpp"

namespace FooGame
{
    class PerspectiveCamera
    {
        public:
            PerspectiveCamera();
            const glm::vec3 GetPosition() const { return m_Position; }
            const glm::mat4 GetProjection() const { return m_Projection; }
            const glm::mat4& GetView() const { return m_View; }
            const glm::mat4 GetModel() const
            {
                return glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
                                   glm::vec3{0.0f, 0.0f, 1.0f});
            }
            void SetAspect(float aspect);
            void RecalculateViewMatrix();
            void GoForward();
            void GoBackward();
            void GoLeft();
            void GoRight();
            void GoUp();
            void GoDown();

            void TurnRight();
            void TurnLeft();

            void LeanForward();
            void LeanBackward();
            float m_Zoom = 90.0f;

        private:
        private:
            glm::mat4 m_View       = glm::mat4(1.0f);
            glm::mat4 m_Projection = glm::mat4(1.0f);
            glm::vec3 m_Position   = glm::vec3{2.0f, 2.0f, 2.0f};
            glm::vec3 m_Up         = {0.0f, 0.0f, 1.0f};
            float m_Aspect         = 16.0f / 9.0f;
    };

}  // namespace FooGame
