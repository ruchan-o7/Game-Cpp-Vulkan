
#include "PerspectiveCamera.h"
#include "../Core/Base.h"
#include "glm/geometric.hpp"
namespace FooGame
{

    PerspectiveCamera::PerspectiveCamera()
    {
        RecalculateViewMatrix();
    }
    void PerspectiveCamera::RecalculateViewMatrix()
    {
        {
            glm::vec3 dir = {};
            dir.x       = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
            dir.y       = sin(glm::radians(m_Pitch));
            dir.z       = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
            m_Direction = glm::normalize(dir);
        }
        m_Right = glm::normalize(glm::cross(m_Direction, m_WorldUp));
        m_Up    = glm::normalize(glm::cross(m_Right, m_Direction));

        m_View       = glm::lookAt(m_Position, m_Position + m_Direction, m_Up);
        m_Projection = glm::perspective(glm::radians(m_Zoom), m_Aspect,
                                        m_NearClip, m_FarClip);
        m_Projection[1][1] *= -1.0f;
    }
    void PerspectiveCamera::SetAspect(float aspect)
    {
        m_Aspect = aspect;
    }
    void PerspectiveCamera::Zoom(float amount)
    {
        m_Zoom += amount;
        if (m_Zoom <= 20)
        {
            m_Zoom = 20;
        }
        if (m_Zoom >= 120)
        {
            m_Zoom = 120;
        }
    }
    const glm::mat4 PerspectiveCamera::GetModel() const
    {
        return glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
                           glm::vec3{0.0f, 0.0f, 1.0f});
    }

    void PerspectiveCamera::SetPosition(glm::vec3 newPos)
    {
        m_Position = newPos;
        RecalculateViewMatrix();
    }
    void PerspectiveCamera::GoForward()
    {
        m_Position += glm::vec3{0.0f, 1.0f, 0.0f} * m_Direction;
    }
    void PerspectiveCamera::GoBackward()
    {
        m_Position -= glm::vec3{0.0f, 1.0f, 0.0f} * m_Direction;
    }
    void PerspectiveCamera::GoLeft()
    {
        m_Position -= glm::normalize(glm::cross(m_Direction, m_Up));
    }
    void PerspectiveCamera::GoRight()
    {
        m_Position += glm::normalize(glm::cross(m_Direction, m_Up));
    }
    void PerspectiveCamera::GoUp()
    {
        m_Position += glm::vec3(0.0f, 0.0f, 0.2f);
    }
    void PerspectiveCamera::GoDown()
    {
        m_Position -= glm::vec3(0.0f, 0.0f, 0.2f);
    }
    void PerspectiveCamera::Look(Tuple<int, int> offset)
    {
        m_Yaw   += offset.second;
        m_Pitch += offset.first;
        if (m_Pitch > 89.0f)
        {
            m_Pitch = 89.0f;
        }
        if (m_Pitch < -89.0f)
        {
            m_Pitch = -89.0f;
        }
    }

}  // namespace FooGame
