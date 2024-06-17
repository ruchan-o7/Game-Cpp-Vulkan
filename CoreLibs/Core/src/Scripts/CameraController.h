#pragma once
#include "../Scene/ScriptableEntity.h"

namespace FooGame
{
    class Camera;
    class TransformComponent;
    class CameraController : public ScriptableEntity
    {
            void OnUpdate(float ts) override;
            void OnCreate() override;

        private:
            Camera* m_pCamera;
            TransformComponent* m_pTransform;
    };
}  // namespace FooGame