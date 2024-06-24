#pragma once
#include "../Scene/ScriptableEntity.h"
namespace FooGame
{
    struct TransformComponent;
    namespace Script
    {
        class RotateScript : public ScriptableEntity
        {
                void OnUpdate(float ts) override;
                void OnCreate() override;

            private:
                TransformComponent* m_Transform;
        };
    }  // namespace Script

    REGISTER_SCRIPT(RotateScript)
}  // namespace FooGame
