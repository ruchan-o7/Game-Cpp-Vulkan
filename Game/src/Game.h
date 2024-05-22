#pragma once
#include <Engine.h>
#include <Core.h>
#include <vector>
namespace FooGame
{
    class Game
    {
        public:
            Game();
            ~Game();
            void Run();

        private:
            std::vector<Scene*> m_Scenes;
            WindowsWindow* m_Window;
            PerspectiveCamera m_Camera;

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
