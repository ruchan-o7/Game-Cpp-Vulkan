#pragma once
#include "../Scene/ScriptableEntity.h"
namespace FooGame
{
    struct TransformComponent;
    class RotateScript : public ScriptableEntity
    {
            void OnUpdate(float ts) override;
            void OnCreate() override;

        private:
            TransformComponent* m_Transform;
    };
}  // namespace FooGame
