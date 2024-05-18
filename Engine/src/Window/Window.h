#pragma once
#include "../Defines.h"
#include "../Events/Event.h"
#include <functional>

struct GLFWwindow;
namespace Engine
{
    struct WindowProperties
    {
            String Title;
            u32 Width;
            u32 Height;
            WindowProperties(const String& title = "Game", u32 width = 1600,
                             u32 height = 900)
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
            void Run();
            void Close();
            GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }
            void SetOnEventFunction(const EventCallback& callback)
            {
                m_Data.EventCallback = callback;
            }
            const u32 GetWidth() const { return m_Data.Width; }
            const u32 GetHeight() const { return m_Data.Height; }
            bool SetVsync(bool enabled);
            void PollEvents();
            void SetWindowTitle(const char* title);
            void SetCursorCenter();
            bool ShouldClose();
            double GetTime();
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
                    String Title;
                    int Width, Height;
                    bool VSync;
                    double CursorPosX, CursorPosY;

                    EventCallback EventCallback;
            };
            WindowData m_Data;
    };
}  // namespace Engine
