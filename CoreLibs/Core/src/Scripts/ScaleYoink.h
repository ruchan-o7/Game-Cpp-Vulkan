#pragma once
#include "../Scene/ScriptableEntity.h"
namespace FooGame
{
    struct TransformComponent;
    class ScaleYoink : public ScriptableEntity
    {
        public:
            void OnCreate() override;
            void OnUpdate(float ts) override;

        private:
            TransformComponent* m_Transform;
    };

}  // namespace FooGame
