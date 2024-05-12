#pragma once
#include <Core/Core/Window.h>
#include <Core/Events/Event.h>
#include <Core/Events/ApplicationEvent.h>
#include <Core/Events/KeyEvent.h>
#include <Core/Core/Engine.h>
#include "Core/Core/OrthographicCamera.h"
#include "Core/Core/PerspectiveCamera.h"
#include "Core/Events/MouseMovedEvent.h"
#include "Core/Graphics/Renderer2D.h"
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
            Shared<Texture2D> m_Tex;
            float m_Tilin    = 1.0f;
            glm::vec4 m_Tint = {1.0f, 1.0f, 1.0f, 1.0f};

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
