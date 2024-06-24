#include "OrthographicCamera.h"
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include "glm/gtx/quaternion.hpp"

namespace FooGame
{

    void OrthographicCamera::RecalculateViewMatrix()
    {
        constexpr int frustum  = 1000;
        float aspect           = 16.0f / 9.0f;
        m_Projection           = glm::ortho(m_Left, m_Right, m_Bottom, m_Top);
        m_Projection[1][1]    *= -1.0f;
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), m_Position) *
            glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        m_View     = transform;
        m_ViewProj = m_Projection * m_View;
    }

    OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
        : m_View(1.0f), m_Left(left), m_Right(right), m_Bottom(bottom), m_Top(top)
    {
        m_Projection = glm::ortho(left, right, bottom, top);
        m_ViewProj   = m_Projection * m_View;
    }
    void OrthographicCamera::SetProj(float left, float right, float bottom, float top)
    {
        m_Top    = top;
        m_Bottom = bottom;
        m_Right  = right;
        m_Left   = left;
        RecalculateViewMatrix();
    }
}  // namespace FooGame
