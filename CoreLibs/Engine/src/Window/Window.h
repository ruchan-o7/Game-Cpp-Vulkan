#pragma once
#include <cstdint>
#include <functional>
#include <string>

struct GLFWwindow;
namespace FooGame
{
    class Event;
    struct WindowProperties
    {
            std::string Title;
            uint32_t Width;
            uint32_t Height;
            WindowProperties(const std::string& title = "Game",
                             uint32_t width = 1600, uint32_t height = 900)
                : Title(title), Width(width), Height(height)
            {
            }
    };
    class Window
    {
            using EventCallback = std::function<void(Event&)>;

        public:
            Window(WindowProperties specifications = WindowProperties());
            ~Window();
            static Window& Get();
            static double GetTime();
            void Run();
            void Close();
            GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }
            void SetOnEventFunction(const EventCallback& callback)
            {
                m_Data.EventCallback = callback;
            }
            const uint32_t GetWidth() const { return m_Data.Width; }
            const uint32_t GetHeight() const { return m_Data.Height; }
            bool SetVsync(bool enabled);
            void PollEvents();
            void SetWindowTitle(const char* title);
            void SetCursorCenter();
            bool ShouldClose();
            double GetCursorPosX() const;
            double GetCursorPosY() const;
            inline void WaitEvent();

        private:
            void Init(const WindowProperties& props);
            void Shutdown();

        private:
            GLFWwindow* m_WindowHandle = nullptr;
            struct WindowData
            {
                    std::string Title;
                    int Width, Height;
                    bool VSync;
                    double CursorPosX, CursorPosY;

                    EventCallback EventCallback;
            };
            WindowData m_Data;
    };
}  // namespace FooGame
