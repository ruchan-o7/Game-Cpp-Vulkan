#pragma once
#include "../Defines.h"
#include <glm/glm.hpp>

namespace FooGame
{
    class OrthographicCamera
    {
        public:
            OrthographicCamera(float left, float right, float bottom,
                               float top);

            const glm::vec3 GetPosition() const { return m_Position; }
            const glm::mat4 GetProjection() const { return m_Projection; }
            const glm::mat4& GetView() const { return m_View; }
            const glm::mat4& GetViewProj() const { return m_ViewProj; }
            void SetPosition(glm::vec3 newPos)
            {
                m_Position = newPos;
                RecalculateViewMatrix();
            }
            void SetProj(float left, float right, float bottom, float top);
            float m_Left, m_Right, m_Bottom, m_Top;
            void RecalculateViewMatrix();

        private:
            glm::mat4 m_View       = glm::mat4(1.0f);
            glm::mat4 m_Projection = glm::mat4(1.0f);
            glm::mat4 m_ViewProj   = glm::mat4(1.0f);

            glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
            float m_Rotation     = 0.0f;
    };
}  // namespace FooGame
