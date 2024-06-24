#pragma once
#include "../Scene/ScriptableEntity.h"
#include "src/Scene/Component.h"
#include "src/Scene/ScriptableEntity.h"

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

            private:
                Camera* m_pCamera;
                TransformComponent* m_pTransform;
        };
    }  // namespace Script
    REGISTER_SCRIPT(CameraController)

}  // namespace FooGame
