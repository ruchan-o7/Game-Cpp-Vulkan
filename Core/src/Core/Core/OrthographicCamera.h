#pragma once
#include "Base.h"
#include "Core/Core/Window.h"

namespace FooGame
{
    class OrthographicCamera
    {
        public:
            OrthographicCamera(glm::vec3 pos = {0.0f, 0.0f, 0.0f});

            const glm::vec3 GetPosition() const { return m_Position; }
            const glm::mat4 GetProjection() const { return m_Projection; }
            const glm::mat4& GetView() const { return m_View; }
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
            void GoLeft();
            void GoRight();
            void GoUp();
            void GoDown();
            void Look(Tuple<int, int> offset);

        private:
            void RecalculateViewMatrix();

        private:
            glm::vec3 m_Position;

            glm::vec3 m_Up      = {0.0f, 0.0f, 1.0f};
            glm::vec3 m_WorldUp = {0.0f, 0.0f, 1.0f};
            glm::vec3 m_Right   = {0.0f, 1.0f, 0.0f};

            glm::mat4 m_View       = glm::mat4(1.0f);
            glm::mat4 m_Projection = glm::mat4(1.0f);

            glm::vec3 m_Direction = {0.0f, 1.0f, 0.0f};
            float m_Rotation;
    };
}  // namespace FooGame
