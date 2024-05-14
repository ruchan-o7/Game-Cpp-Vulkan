#pragma once
#include <Core/Core/Window.h>
#include <Core/Events/Event.h>
#include <Core/Events/ApplicationEvent.h>
#include <Core/Events/KeyEvent.h>
#include <Core/Core/Engine.h>
#include "Core/Events/MouseMovedEvent.h"
#include "World/Level.h"
namespace FooGame
{
    class Game
    {
        public:
            Game();
            ~Game();
            void Run();

        private:
            List<Level*> m_Levels;
            WindowsWindow* m_Window;

        private:
            float m_DeltaTime = 0.01f;

        private:
            void Init();
            void Shutdown();
            void OnEvent(Event& e);
            bool OnKeyEvent(KeyPressedEvent& key);
            bool OnWindowResized(WindowResizeEvent& event);

            bool OnMouseMoved(MouseMovedEvent& event);
            bool OnMouseScroll(MouseScrolledEvent& event);
            bool OnMousePressed(MouseButtonPressedEvent& event);
            bool OnMouseRelease(MouseButtonReleasedEvent& event);
    };

}  // namespace FooGame
