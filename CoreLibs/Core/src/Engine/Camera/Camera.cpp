#include "Camera.h"
namespace FooGame
{
    void Camera::UpdateMatrix()
    {
        if (type == Type::FirstPerson)
        {
            glm::vec3 camFront;
            camFront.x = -cos(glm::radians(Rotation.x)) * sin(glm::radians(Rotation.y));
            camFront.y = sin(glm::radians(Rotation.x));
            camFront.z = cos(glm::radians(Rotation.x)) * cos(glm::radians(Rotation.y));
            Front      = glm::normalize(camFront);
        }
        glm::mat4 currentMatrix = View;

        glm::mat4 rotM = glm::mat4(1.0f);
        glm::mat4 transM;

        rotM = glm::rotate(rotM, glm::radians(Rotation.x * (FlipY ? -1.0f : 1.0f)),
                           glm::vec3(1.0f, 0.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 translation = Position;
        if (FlipY)
        {
            translation.y *= -1.0f;
        }
        transM = glm::translate(glm::mat4(1.0f), translation);

        if (type == Type::FirstPerson)
        {
            View = rotM * transM;
        }
        else
        {
            View = transM * rotM;
        }
        Perspective = glm::perspective(glm::radians(Fov), Aspect, ZNear, ZFar);
        if (FlipY)
        {
            Perspective[1][1] *= -1.0f;
        }
    }
}  // namespace FooGame
