#include "OrthographicCamera.h"

namespace FooGame
{

    void OrthographicCamera::RecalculateViewMatrix()
    {
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), m_Position) *
            glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation),
                        glm::vec3(0.0f, 0.0f, 1.0f));
        m_Projection = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, 0.1f, 1000.f);
    }

    OrthographicCamera::OrthographicCamera(glm::vec3 pos) : m_Position(pos)
    {
        RecalculateViewMatrix();
    }
    void OrthographicCamera::GoLeft()
    {
        m_Position -= glm::normalize(glm::cross(m_Direction, m_Up));
    }
    void OrthographicCamera::GoRight()
    {
        m_Position += glm::normalize(glm::cross(m_Direction, m_Up));
    }
    void OrthographicCamera::GoUp()
    {
        m_Position += glm::vec3(0.0f, 0.0f, 0.2f);
    }
    void OrthographicCamera::GoDown()
    {
        m_Position -= glm::vec3(0.0f, 0.0f, 0.2f);
    }
    void OrthographicCamera::Look(Tuple<int, int> offset)
    {
        // m_Yaw   += offset.second;
        // m_Pitch += offset.first;
        // if (m_Pitch > 89.0f)
        // {
        //     m_Pitch = 89.0f;
        // }
        // if (m_Pitch < -89.0f)
        // {
        //     m_Pitch = -89.0f;
        // }
    }
}  // namespace FooGame
