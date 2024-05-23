#pragma once
#include <cstdint>
#include "../Events/Event.h"
#include <functional>

struct GLFWwindow;
namespace FooGame
{
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
    class WindowsWindow
    {
            using EventCallback = std::function<void(Event&)>;

        public:
            WindowsWindow(WindowProperties specifications = WindowProperties());
            ~WindowsWindow();
            static WindowsWindow& Get();
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
