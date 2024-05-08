#include "Camera.h"
#define GLM_FORCE_RADIANS
#include "../Core/Base.h"

namespace FooGame
{

    void Camera::RecalculateViewMatrix()
    {
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), m_Position) *
            glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation),
                        glm::vec3(0.0f, 0.0f, 1.0f));
        m_View       = glm::inverse(transform);
        m_Projection = glm::perspective(glm::radians(45.0f), 1600.0f / 900.0f,
                                        0.1f, 1000.0f);
    }

}  // namespace FooGame
