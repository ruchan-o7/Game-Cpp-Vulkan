#include "OrthographicCamera.h"

namespace FooGame
{

    void OrthographicCamera::RecalculateViewMatrix()
    {
        m_Projection        = glm::ortho(m_Left, m_Rotation, m_Bottom, m_Top);
        m_Projection[1][1] *= -1.0f;
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), m_Position) *
            glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation),
                        glm::vec3(0.0f, 0.0f, 1.0f));
        m_View = transform;
        // m_View[1][1] *= -1.0f;
        m_ViewProj = m_Projection * m_View;
    }

    OrthographicCamera::OrthographicCamera(float left, float right,
                                           float bottom, float top)
        : m_View(1.0f),
          m_Left(left),
          m_Right(right),
          m_Bottom(bottom),
          m_Top(top)
    {
        m_Projection = glm::ortho(left, right, bottom, top);
        m_ViewProj   = m_Projection * m_View;
        // RecalculateViewMatrix();
    }
    void OrthographicCamera::SetProj(float left, float right, float bottom,
                                     float top)
    {
        m_Top    = top;
        m_Bottom = bottom;
        m_Right  = right;
        m_Left   = left;
        RecalculateViewMatrix();
    }
    void OrthographicCamera::GoLeft()
    {
    }
    void OrthographicCamera::GoRight()
    {
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
