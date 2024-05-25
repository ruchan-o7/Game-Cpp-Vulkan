#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace FooGame
{
    class Camera
    {
        public:
            enum CameraType
            {
                lookat,
                firstperson
            };
            CameraType type = CameraType::lookat;

            glm::vec3 rotation       = glm::vec3();
            glm::vec3 position       = glm::vec3();
            glm::vec4 viewPos        = glm::vec4();
            glm::vec2 LastMouseState = glm::vec2();

            float rotationSpeed = 0.01f;
            float movementSpeed = 0.01f;

            float fov;
            float znear, zfar;
            bool updated = true;
            bool flipY   = false;
            struct
            {
                    glm::mat4 perspective;
                    glm::mat4 view;
            } matrices;

            struct
            {
                    bool left  = false;
                    bool right = false;
                    bool up    = false;
                    bool down  = false;
            } keys;

            bool moving()
            {
                return keys.left || keys.right || keys.up || keys.down;
            }
            void updateViewMatrix();

            float GetNearClip() const { return znear; }

            float GetFarClip() const { return zfar; }

            void setPerspective(float fov, float aspect, float znear,
                                float zfar);

            void updateAspectRatio(float aspect);

            void setPosition(glm::vec3 position)
            {
                this->position = position;
                updateViewMatrix();
            }

            void setRotation(glm::vec3 rotation)
            {
                this->rotation = rotation;
                updateViewMatrix();
            }

            void rotate(glm::vec3 delta)
            {
                this->rotation += delta;
                updateViewMatrix();
            }

            void setTranslation(glm::vec3 translation)
            {
                this->position = translation;
                updateViewMatrix();
            };

            void translate(glm::vec3 delta)
            {
                this->position += delta;
                updateViewMatrix();
            }

            void setRotationSpeed(float rotationSpeed)
            {
                this->rotationSpeed = rotationSpeed;
            }

            void setMovementSpeed(float movementSpeed)
            {
                this->movementSpeed = movementSpeed;
            }

            void update(float deltaTime);
    };
}  // namespace FooGame
