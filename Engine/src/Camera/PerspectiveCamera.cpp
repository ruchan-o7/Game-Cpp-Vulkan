#include "PerspectiveCamera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
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

        m_View = glm::lookAt(m_Position, m_Position + m_Direction, m_Up);

        m_Projection        = glm::perspective(glm::radians(m_Zoom), m_Aspect,
                                               m_NearClip, m_FarClip);
        m_Projection[1][1] *= -1;
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

    void PerspectiveCamera::SetPosition(glm::vec3 newPos)
    {
        m_Position = newPos;
        RecalculateViewMatrix();
    }

}  // namespace FooGame
