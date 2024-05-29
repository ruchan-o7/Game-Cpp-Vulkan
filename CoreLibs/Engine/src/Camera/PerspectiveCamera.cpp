#include "PerspectiveCamera.h"

#include <glm/gtx/quaternion.hpp>
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
    void PerspectiveCamera::Rotate(glm::vec2 delta)
    {
        float yawSign  = GetUpDirection().y < 0 ? 1.0f : -1.0f;
        m_Yaw         += yawSign * delta.y * 10.0f;
        m_Pitch       += delta.x * 10.0f;
    }
    glm::vec3 PerspectiveCamera::GetUpDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    glm::quat PerspectiveCamera::GetOrientation() const
    {
        return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
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

    void PerspectiveCamera::SetProjection(const glm::mat4& projection)
    {
        m_Projection = projection;
    }
    void PerspectiveCamera::SetPosition(const glm::vec3& newPos)
    {
        m_Position = newPos;
        RecalculateViewMatrix();
    }

}  // namespace FooGame
