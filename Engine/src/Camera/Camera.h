#pragma once
#include <glm/glm.hpp>
namespace FooGame
{
    class Camera
    {
        public:
            Camera() = default;
            Camera(const glm::mat4& projection) : m_Projection(projection) {}

            virtual ~Camera() = default;

            const glm::vec3 GetPosition() const { return m_Position; }
            const glm::mat4& GetView() const { return m_View; }

            const glm::mat4& GetProjection() const { return m_Projection; }
            virtual void SetProjection(const glm::mat4& projection)
            {
                m_Projection = projection;
            }
            void SetPosition(const glm::vec3& newPos)
            {
                m_Position = newPos;
                RecalculateViewMatrix();
            }
            virtual void RecalculateViewMatrix() {}

        protected:
            glm::mat4 m_Projection = glm::mat4(1.0f);
            glm::mat4 m_View       = glm::mat4(1.0f);
            glm::vec3 m_Position   = glm::vec3{0.0f, 0.0f, 0.0f};
    };
}  // namespace FooGame
