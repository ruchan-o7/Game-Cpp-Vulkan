#pragma once

#include "Core/Level/Component.h"
namespace FooGame
{

    class GameObject
    {
        public:
            GameObject()                  = default;
            GameObject(const GameObject&) = default;
            TransformComponent Transform;
    };
}  // namespace FooGame
