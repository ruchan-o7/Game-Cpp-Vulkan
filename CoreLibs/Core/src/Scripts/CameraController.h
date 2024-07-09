#pragma once
#include "../Scene/ScriptableEntity.h"

namespace FooGame
{
    class Camera;
    class TransformComponent;

    namespace Script
    {
        class CameraController : public ScriptableEntity
        {
                void OnUpdate(float ts) override;
                void OnCreate() override;

                float RotationSpeed = 1.0f, MovementSpeed = 1.0f;
                float Fov, AspectRatio;
                bool FlipY = true;

            private:
                Camera* m_pCamera;
                TransformComponent* m_pTransform;
                glm::mat4 m_View, m_Projection;
        };
    }  // namespace Script
    REGISTER_SCRIPT(CameraController)

}  // namespace FooGame
