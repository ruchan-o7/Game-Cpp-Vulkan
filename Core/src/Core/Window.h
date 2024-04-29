#pragma once
#include "Defines.h"
struct GLFWwindow;
namespace FooGame
{

    struct GameSpecifications
    {
            u32 width  = 1600;
            u32 height = 900;
    };
    class WindowsWindow
    {
        public:
            WindowsWindow(
                GameSpecifications specifications = GameSpecifications());
            ~WindowsWindow();
            static WindowsWindow& Get();
            void Run();
            void Close();

        private:
            void Init();
            void Shutdown();

        private:
            GameSpecifications m_Specification;
            GLFWwindow* m_WindowHandle = nullptr;
            bool m_Running             = false;

            float m_TimeStep      = 0.0f;
            float m_FrameTime     = 0.0f;
            float m_LastFrameTime = 0.0f;
    };
}  // namespace FooGame
