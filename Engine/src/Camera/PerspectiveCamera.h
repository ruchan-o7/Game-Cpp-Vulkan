#pragma once
#include <glm/glm.hpp>
namespace Engine
{
    class PerspectiveCamera
    {
        public:
            PerspectiveCamera();
            const glm::vec3 GetPosition() const { return m_Position; }
            const glm::mat4 GetProjection() const { return m_Projection; }
            const glm::mat4& GetView() const { return m_View; }
            const glm::mat4 GetModel() const;
            void SetPosition(glm::vec3 newPos);
            void SetAspect(float aspect);
            void RecalculateViewMatrix();
            void GoForward();
            void GoBackward();
            void GoLeft();
            void GoRight();
            void GoUp();
            void GoDown();
            void Look(double offsetX, double offsetY);
            void Zoom(float amount = 10);
            float m_Zoom = 90.0f;

        private:
            float m_Yaw      = 90.0f;
            float m_Pitch    = 0.0f;
            float m_NearClip = 0.0001f;
            float m_FarClip  = 1000.0f;
            float m_Aspect   = 16.0f / 9.0f;

        private:
            glm::mat4 m_View       = glm::mat4(1.0f);
            glm::mat4 m_Projection = glm::mat4(1.0f);
            glm::vec3 m_Position   = glm::vec3{0.0f, 0.0f, 0.0f};

            glm::vec3 m_Up      = {0.0f, 0.0f, 1.0f};
            glm::vec3 m_WorldUp = {0.0f, 0.0f, 1.0f};
            glm::vec3 m_Right   = {0.0f, 1.0f, 0.0f};
            // glm::vec3 m_Right     = {0.0f, 0.0f, 1.0f};
            glm::vec3 m_Direction = {0.0f, 1.0f, 0.0f};
    };

}  // namespace Engine
