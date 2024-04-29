#include "./Window.h"
#include <utility>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Events/KeyEvent.h"
#include "src/Core/Input/KeyCodes.h"
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
        // TODO: Init renderer
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
    }
    void WindowsWindow::Run()
    {
        while (!glfwWindowShouldClose(m_WindowHandle))
        {
            glfwPollEvents();
        }
    }
    void WindowsWindow::Shutdown()
    {
        // TODO: Renderer shutdown
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
