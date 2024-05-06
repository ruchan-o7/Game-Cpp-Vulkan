#pragma once
#include "../Core/Base.h"
namespace FooGame
{

    class Camera
    {
        public:
            Camera() = default;
            Camera(const glm::mat4& proj)
                : m_Model(proj),
                  m_View(proj),
                  m_Projection(proj),
                  m_Position(0.0f)
            {
            }
            ~Camera() = default;

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
            void RecalculateViewMatrix()
            {
                glm::mat4 transform =
                    glm::translate(glm::mat4(1), m_Position) *
                    glm::rotate(glm::mat4(1), glm::radians(m_Rotation),
                                glm::vec3(0, 0, 1));
                m_View = glm::inverse(transform);
            }
            glm::mat4 m_Model;
            glm::mat4 m_View;
            glm::mat4 m_Projection;
            glm::vec3 m_Position;
            float m_Rotation;
    };
}  // namespace FooGame
