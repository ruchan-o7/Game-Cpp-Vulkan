#pragma once
#include <glm/glm.hpp>

namespace Engine
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
            void MoveTo(glm::vec3 pos)
            {
                m_Position = pos;
                RecalculateViewMatrix();
            }
            void SetPosition(glm::vec3 newPos)
            {
                m_Position = newPos;
                RecalculateViewMatrix();
            }
            void SetRotation(float rot)
            {
                m_Rotation = rot;
                RecalculateViewMatrix();
            }
            void SetProj(float left, float right, float bottom, float top);
            void GoLeft();
            void GoRight();
            void GoUp();
            void GoDown();
            void Look(double offsetX, double offsetY);
            float m_Left, m_Right, m_Bottom, m_Top;

        private:
            void RecalculateViewMatrix();

        private:
            glm::mat4 m_View       = glm::mat4(1.0f);
            glm::mat4 m_Projection = glm::mat4(1.0f);
            glm::mat4 m_ViewProj   = glm::mat4(1.0f);

            glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
            float m_Rotation     = 0.0f;
    };
}  // namespace Engine
