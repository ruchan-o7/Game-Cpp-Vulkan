
#include "PerspectiveCamera.h"
namespace FooGame
{

    PerspectiveCamera::PerspectiveCamera()
    {
        RecalculateViewMatrix();
    }
    void PerspectiveCamera::RecalculateViewMatrix()
    {
        m_View = glm::lookAt(m_Position, glm::vec3{0.0f, 0.0f, 0.0f}, m_Up);
        m_Projection =
            glm::perspective(glm::radians(m_Zoom), m_Aspect, 0.001f, 1000.0f);
        m_Projection[1][1] *= -1.0f;
    }
    void PerspectiveCamera::SetAspect(float aspect)
    {
        m_Aspect = aspect;
    }

    void PerspectiveCamera::GoForward()
    {
        m_Position -= glm::vec3{0.0f, 1.0f, 0.0f};
    }
    void PerspectiveCamera::GoBackward()
    {
        m_Position += glm::vec3{0.0f, 1.0f, 0.0f};
        // m_Position += glm::vec3(0.2f, 0.0f, 0.0f);
    }
    void PerspectiveCamera::GoLeft()
    {
        m_Position += glm::vec3{0.0f, -1.0f, 0.0f};
    }
    void PerspectiveCamera::GoRight()
    {
        m_Position -= glm::vec3{0.0f, -1.0f, 0.0f};
    }
    void PerspectiveCamera::TurnRight()
    {
        // m_Yaw += 0.1f;
    }
    void PerspectiveCamera::TurnLeft()
    {
        // m_Yaw -= 0.1f;
    }

    void PerspectiveCamera::LeanForward()
    {
        // m_Pitch += 0.1f;
    }

    void PerspectiveCamera::LeanBackward()
    {
        // m_Pitch -= 0.1f;
    }
    void PerspectiveCamera::GoUp()
    {
        m_Position -= glm::vec3(0.0f, 0.0f, 0.2f);
    }
    void PerspectiveCamera::GoDown()
    {
        m_Position += glm::vec3(0.0f, 0.0f, 0.2f);
    }

}  // namespace FooGame
