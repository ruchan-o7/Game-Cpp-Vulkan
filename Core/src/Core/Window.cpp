#include "Window.h"
#include <utility>
#include <GLFW/glfw3.h>
#include <iostream>
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
        if (!glfwVulkanSupported())
        {
            std::cerr << "Vulkan not supported!\n";
            return;
        }
        // TODO: Init renderer
        m_Running = true;
    }
    void WindowsWindow::Run()
    {
        while (!glfwWindowShouldClose(m_WindowHandle) && m_Running)
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
    WindowsWindow::~WindowsWindow()
    {
        Shutdown();
    }

}  // namespace FooGame
