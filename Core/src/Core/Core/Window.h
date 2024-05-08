#pragma once
#include "../Events/Event.h"
#include "Base.h"
#include "GLFW/glfw3.h"
#include "pch.h"
struct GLFWwindow;
namespace FooGame
{

    struct GameSpecifications
    {
            u32 width  = 1600;
            u32 height = 900;
    };
    struct Vector2
    {
            i32 x = 0, y = 0;
    };
    class WindowsWindow
    {
            using EventCallback = std::function<void(Event&)>;

        public:
            WindowsWindow(
                GameSpecifications specifications = GameSpecifications());
            ~WindowsWindow();
            static WindowsWindow& Get();
            void Run();
            void Close();
            GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }
            void SetOnEventFunction(const EventCallback& callback)
            {
                OnEventCallback = callback;
            }
            EventCallback OnEventCallback;
            void PollEvents();
            void SetWindowTitle(const char* title);
            bool ShouldClose();
            Vector2 GetWindowSize();
            inline void WaitEvent() { glfwWaitEvents(); }

        private:
            void Init();
            void Shutdown();

        private:
            GameSpecifications m_Specification;
            GLFWwindow* m_WindowHandle = nullptr;

            float m_TimeStep      = 0.0f;
            float m_FrameTime     = 0.0f;
            float m_LastFrameTime = 0.0f;
    };
}  // namespace FooGame
