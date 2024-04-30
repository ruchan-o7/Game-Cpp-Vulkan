#pragma once
#include <Core/Window.h>
#include <Core/Events/Event.h>
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/KeyEvent.h"
namespace FooGame
{
    class Game
    {
        public:
            Game();
            ~Game();
            void Run();

        private:
            WindowsWindow* m_Window;

        private:
            void Init();
            void Shutdown();
            void OnEvent(Event& e);
            bool OnKeyEvent(KeyPressedEvent& key);
            bool OnWindowResized(WindowResizeEvent& event);
    };

}  // namespace FooGame
