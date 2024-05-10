#include "./Window.h"
#include <GLFW/glfw3.h>
#include "../Events/KeyEvent.h"
#include "../Events/ApplicationEvent.h"
#include "../Input/KeyCodes.h"
#include "../Events/MouseMovedEvent.h"
#include "pch.h"
namespace FooGame
{
    static WindowsWindow* s_Instance = nullptr;
    WindowsWindow& WindowsWindow::Get()
    {
        return *s_Instance;
    }
    WindowsWindow::WindowsWindow(GameSpecifications specifications)
        : m_Specification(std::move(specifications))
    {
        s_Instance = this;
        Init();
    }

    double WindowsWindow::GetTime()
    {
        return glfwGetTime();
    }
    Vector2 WindowsWindow::GetWindowSize()
    {
        i32 w, h;
        glfwGetFramebufferSize(m_WindowHandle, &w, &h);
        return {w, h};
    }
    void WindowsWindow::SetWindowTitle(const char* title)
    {
        glfwSetWindowTitle(m_WindowHandle, title);
    }
    void WindowsWindow::Init()
    {
        if (!glfwInit())
        {
            std::cerr << "GLFW COULD NOT INIT\n";
            const char* message;
            glfwGetError(&message);
            std::cout << message << std::endl;
            return;
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_WindowHandle =
            glfwCreateWindow(m_Specification.width, m_Specification.height,
                             "GAME", nullptr, nullptr);
        glfwSetWindowUserPointer(m_WindowHandle, this);
        if (!glfwVulkanSupported())
        {
            std::cerr << "Vulkan not supported!\n";
            return;
        }
        glfwSetKeyCallback(
            m_WindowHandle,
            [](GLFWwindow* window, int key, int scancode, int action, int mods)
            {
                WindowsWindow& data =
                    *(WindowsWindow*)glfwGetWindowUserPointer(window);
                switch (action)
                {
                    case GLFW_PRESS:
                    {
                        KeyPressedEvent event((KeyCode)key, 0);
                        data.OnEventCallback(event);
                        break;
                    }
                    case GLFW_RELEASE:
                    {
                        KeyReleasedEvent event((KeyCode)key);
                        data.OnEventCallback(event);
                        break;
                    }
                    case GLFW_REPEAT:
                    {
                        KeyPressedEvent event((KeyCode)key, true);
                        data.OnEventCallback(event);
                        break;
                    }
                }
            });
        glfwSetFramebufferSizeCallback(
            m_WindowHandle,
            [](GLFWwindow* window, int width, int height)
            {
                WindowsWindow& data =
                    *(WindowsWindow*)glfwGetWindowUserPointer(window);
                data.m_Specification.width  = width;
                data.m_Specification.height = height;
                WindowResizeEvent e{static_cast<unsigned int>(width),
                                    static_cast<unsigned int>(height)};
                data.OnEventCallback(e);
            });
        glfwSetMouseButtonCallback(
            m_WindowHandle,
            [](GLFWwindow* window, int button, int action, int mods)
            {
                WindowsWindow& data =
                    *(WindowsWindow*)glfwGetWindowUserPointer(window);
                switch (action)
                {
                    case GLFW_PRESS:
                    {
                        MouseButtonPressedEvent e{
                            static_cast<MouseCode>(button)};
                        data.OnEventCallback(e);
                        break;
                    }
                    case GLFW_RELEASE:
                    {
                        MouseButtonReleasedEvent e{
                            static_cast<MouseCode>(button)};
                        data.OnEventCallback(e);
                        break;
                    }
                }
            });
        glfwSetCursorPosCallback(
            m_WindowHandle,
            [](GLFWwindow* window, double xPos, double yPos)
            {
                WindowsWindow& data =
                    *(WindowsWindow*)glfwGetWindowUserPointer(window);
                MouseMovedEvent e{static_cast<float>(xPos),
                                  static_cast<float>(yPos)};
                data.OnEventCallback(e);
            });
        glfwSetScrollCallback(
            m_WindowHandle,
            [](GLFWwindow* window, double xOffset, double yOffset)
            {
                WindowsWindow& data =
                    *(WindowsWindow*)glfwGetWindowUserPointer(window);
                MouseScrolledEvent e{static_cast<float>(xOffset),
                                     static_cast<float>(yOffset)};
                data.OnEventCallback(e);
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
    Tuple<int, int> WindowsWindow::GetCursorPos()
    {
        double x, y;
        glfwGetCursorPos(m_WindowHandle, &x, &y);
        return {static_cast<int>(x), static_cast<int>(y)};
    }
    void WindowsWindow::SetCursorCenter()
    {
        glfwSetCursorPos(m_WindowHandle, m_Specification.width / 2,
                         m_Specification.height / 2);
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

}  // namespace FooGame
