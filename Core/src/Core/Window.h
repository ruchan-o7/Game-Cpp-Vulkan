#pragma once
#include "Events/Event.h"
#include "Defines.h"
#include <functional>
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
            bool ShouldClose();

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
