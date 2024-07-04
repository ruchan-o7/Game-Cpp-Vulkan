#pragma once
#include "../Scene/ScriptableEntity.h"
namespace FooGame
{
    struct TransformComponent;
    namespace Script
    {

        class ScaleYoink : public ScriptableEntity
        {
            public:
                void OnCreate() override;
                void OnUpdate(float ts) override;

            private:
                TransformComponent* m_Transform;
        };

        REGISTER_SCRIPT(ScaleYoink)

    }  // namespace Script

}  // namespace FooGame
