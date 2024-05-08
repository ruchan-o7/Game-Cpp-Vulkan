#pragma once
#include "Base.h"

namespace FooGame
{
    class OrthographicCamera
    {
        public:
            OrthographicCamera(glm::vec3 pos);

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
            glm::vec3 m_Position;
            glm::vec3 m_Front;
            glm::vec3 m_Up;

            glm::mat4 m_Model      = glm::mat4(1.0f);
            glm::mat4 m_View       = glm::mat4(1.0f);
            glm::mat4 m_Projection = glm::mat4(1.0f);

            float m_Rotation;
    };
}  // namespace FooGame
