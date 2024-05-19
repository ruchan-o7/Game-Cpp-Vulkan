#pragma once
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

        private:
            WindowsWindow* m_Window;
    };
}  // namespace FooGame
