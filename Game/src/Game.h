#pragma once
#include <Core/Core/Window.h>
#include <Core/Events/Event.h>
#include <Core/Events/ApplicationEvent.h>
#include <Core/Events/KeyEvent.h>
#include <Core/Core/Engine.h>
#include <thread>
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
            Engine* m_Engine;
            std::thread m_RenderThread;

        private:
            void Init();
            void Shutdown();
            void OnEvent(Event& e);
            bool OnKeyEvent(KeyPressedEvent& key);
            bool OnWindowResized(WindowResizeEvent& event);
    };

}  // namespace FooGame
