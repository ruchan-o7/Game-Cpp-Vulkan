#pragma once
#include "src/Events/Event.h"
namespace FooGame
{
    class WindowsWindow;
    class Editor
    {
        public:
            Editor();
            ~Editor();
            void Run();

        private:
            void Init();
            void OnEvent(Event& event);

        private:
            WindowsWindow* m_Window;
    };
}  // namespace FooGame
