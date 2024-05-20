#pragma once
#include "Camera.h"
namespace FooGame
{
    class PerspectiveCamera : public Camera
    {
        public:
            PerspectiveCamera();
            void SetPosition(glm::vec3 newPos);
            void SetAspect(float aspect);
            void RecalculateViewMatrix() override;
            void Zoom(float amount = 10);
            void Pan(glm::vec2 delta);
            void Rotate(glm::vec2 delta);

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
    };

}  // namespace FooGame
