#pragma once
#include <Core/Core/Window.h>
#include <Core/Events/Event.h>
#include <Core/Events/ApplicationEvent.h>
#include <Core/Events/KeyEvent.h>
#include <Core/Core/Engine.h>
#include "Core/Core/OrthographicCamera.h"
#include "Core/Core/PerspectiveCamera.h"
#include "Core/Events/MouseMovedEvent.h"
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
            i32 m_BenchmarkAmount = 100;
            PerspectiveCamera m_Camera;
            bool m_SecondMouse = false;

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
