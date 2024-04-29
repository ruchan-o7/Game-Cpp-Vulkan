#pragma once
#include <Core/Window.h>
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
    };

}  // namespace FooGame
