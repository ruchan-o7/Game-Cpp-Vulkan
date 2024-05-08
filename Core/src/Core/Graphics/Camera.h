#pragma once
#include "../Core/Base.h"
namespace FooGame
{

    class Camera
    {
        public:
            Camera() = default;

            const glm::vec3 GetPosition() const { return m_Position; }
            const glm::mat4 GetProjection() const { return m_Projection; }
            const glm::mat4 GetModel() const
            {
                return glm::translate(m_Model, m_Position);
            }
            const glm::mat4& GetView() const { return m_View; }
            void MoveTo(glm::vec3 pos)
            {
                m_Position = pos;
                RecalculateViewMatrix();
            }

        private:
            void RecalculateViewMatrix();

        private:
            glm::mat4 m_Model      = glm::mat4(1.0f);
            glm::mat4 m_View       = glm::mat4(1.0f);
            glm::mat4 m_Projection = glm::mat4(1.0f);
            glm::vec3 m_Position;
            float m_Rotation;
    };
}  // namespace FooGame
