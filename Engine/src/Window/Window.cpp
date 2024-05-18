#include "Window.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include "../Events/KeyEvent.h"
#include "../Events/ApplicationEvent.h"
#include "../Input/KeyCodes.h"
#include "../Events/MouseMovedEvent.h"
namespace Engine
{
    static WindowsWindow* s_Instance = nullptr;
    WindowsWindow& WindowsWindow::Get()
    {
        return *s_Instance;
    }
    WindowsWindow::WindowsWindow(WindowProperties specifications)
    {
        s_Instance = this;
        Init(specifications);
    }
    inline void WindowsWindow::WaitEvent()
    {
        glfwWaitEvents();
    }

    double WindowsWindow::GetTime()
    {
        return glfwGetTime();
    }
    void WindowsWindow::SetWindowTitle(const char* title)
    {
        m_Data.Title = title;
        glfwSetWindowTitle(m_WindowHandle, title);
    }
    double WindowsWindow::GetCursorPosX() const
    {
        return m_Data.CursorPosX;
    }
    double WindowsWindow::GetCursorPosY() const
    {
        return m_Data.CursorPosY;
    }
    void WindowsWindow::Init(const WindowProperties& props)
    {
        m_Data.Height = props.Height;
        m_Data.Width  = props.Width;
        m_Data.Title  = props.Title;
        if (!glfwInit())
        {
            std::cerr << "GLFW COULD NOT INIT\n";
            const char* message;
            glfwGetError(&message);
            std::cout << message << std::endl;
            return;
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_WindowHandle = glfwCreateWindow(m_Data.Width, m_Data.Height, "GAME",
                                          nullptr, nullptr);
        glfwSetWindowUserPointer(m_WindowHandle, &m_Data);
        if (!glfwVulkanSupported())
        {
            std::cerr << "Vulkan not supported!\n";
            return;
        }
        glfwSetKeyCallback(
            m_WindowHandle,
            [](GLFWwindow* window, int key, int scancode, int action, int mods)
            {
                WindowData& data =
                    *(WindowData*)glfwGetWindowUserPointer(window);
                switch (action)
                {
                    case GLFW_PRESS:
                    {
                        KeyPressedEvent event((KeyCode)key, 0);
                        data.EventCallback(event);
                        break;
                    }
                    case GLFW_RELEASE:
                    {
                        KeyReleasedEvent event((KeyCode)key);
                        data.EventCallback(event);
                        break;
                    }
                    case GLFW_REPEAT:
                    {
                        KeyPressedEvent event((KeyCode)key, true);
                        data.EventCallback(event);
                        break;
                    }
                }
            });
        glfwSetFramebufferSizeCallback(
            m_WindowHandle,
            [](GLFWwindow* window, int width, int height)
            {
                WindowData& data =
                    *(WindowData*)glfwGetWindowUserPointer(window);
                data.Width  = width;
                data.Height = height;
                WindowResizeEvent e{static_cast<unsigned int>(width),
                                    static_cast<unsigned int>(height)};
                data.EventCallback(e);
            });
        glfwSetMouseButtonCallback(
            m_WindowHandle,
            [](GLFWwindow* window, int button, int action, int mods)
            {
                WindowData& data =
                    *(WindowData*)glfwGetWindowUserPointer(window);
                switch (action)
                {
                    case GLFW_PRESS:
                    {
                        MouseButtonPressedEvent e{
                            static_cast<MouseCode>(button)};
                        data.EventCallback(e);
                        break;
                    }
                    case GLFW_RELEASE:
                    {
                        MouseButtonReleasedEvent e{
                            static_cast<MouseCode>(button)};
                        data.EventCallback(e);
                        break;
                    }
                }
            });
        glfwSetCursorPosCallback(
            m_WindowHandle,
            [](GLFWwindow* window, double xPos, double yPos)
            {
                WindowData& data =
                    *(WindowData*)glfwGetWindowUserPointer(window);
                data.CursorPosX = xPos;
                data.CursorPosY = yPos;

                MouseMovedEvent e{static_cast<float>(xPos),
                                  static_cast<float>(yPos)};
                data.EventCallback(e);
            });
        glfwSetScrollCallback(
            m_WindowHandle,
            [](GLFWwindow* window, double xOffset, double yOffset)
            {
                WindowData& data =
                    *(WindowData*)glfwGetWindowUserPointer(window);
                MouseScrolledEvent e{static_cast<float>(xOffset),
                                     static_cast<float>(yOffset)};
                data.EventCallback(e);
            });
    }
    void WindowsWindow::PollEvents()
    {
        glfwPollEvents();
    }
    bool WindowsWindow::ShouldClose()
    {
        return glfwWindowShouldClose(m_WindowHandle);
    }
    void WindowsWindow::SetCursorCenter()
    {
        glfwSetCursorPos(m_WindowHandle, m_Data.Width / 2, m_Data.Height / 2);
    }
    void WindowsWindow::Shutdown()
    {
        glfwDestroyWindow(m_WindowHandle);
        glfwTerminate();
    }
    void WindowsWindow::Close()
    {
        glfwSetWindowShouldClose(m_WindowHandle, true);
    }
    WindowsWindow::~WindowsWindow()
    {
        Shutdown();
    }

}  // namespace Engine
