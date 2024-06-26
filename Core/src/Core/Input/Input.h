#pragma once
#include "KeyCodes.h"
#include <glm/glm.hpp>

namespace FooGame
{
    class Input
    {
        public:
            static bool IsKeyDown(KeyCode keycode);
            static bool IsMouseButtonDown(MouseButton keycode);
            static glm::vec2 GetMousePosition();
            static void SetCursorMode(CursorMode mode);
    };

}  // namespace FooGame
