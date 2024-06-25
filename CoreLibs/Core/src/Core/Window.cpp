#include "Window.h"
#include "../Events/KeyEvent.h"
#include "../Events/ApplicationEvent.h"
#include "GLFW/glfw3.h"
#include "../Input/MouseCodes.h"
#include "../Events/MouseEvent.h"
#include <Log.h>
namespace FooGame
{
    static Window* s_Instance = nullptr;
    static void GLFWErrorCallback(int err, const char* desc)
    {
        FOO_CORE_ERROR("GLFW Error ({0}): {1}", err, desc);
    }
    Window& Window::Get()
    {
        return *s_Instance;
    }
    double Window::GetTime()
    {
        return glfwGetTime();
    }
    HWND Window::GetWin32NativeHandle() const
    {
        return glfwGetWin32Window(m_WindowHandle);
    }

    Window::Window(WindowProperties specifications)
    {
        s_Instance = this;
        Init(specifications);
    }
    inline void Window::WaitEvent()
    {
        glfwWaitEvents();
    }

    void Window::SetWindowTitle(const char* title)
    {
        m_Data.Title = title;
        glfwSetWindowTitle(m_WindowHandle, title);
    }
    double Window::GetCursorPosX() const
    {
        return m_Data.CursorPosX;
    }
    double Window::GetCursorPosY() const
    {
        return m_Data.CursorPosY;
    }
    void Window::Init(const WindowProperties& props)
    {
        FOO_ENGINE_TRACE("Window creating");
        m_Data.Height = props.Height;
        m_Data.Width  = props.Width;
        m_Data.Title  = props.Title;
        if (!glfwInit())
        {
            FOO_ENGINE_CRITICAL("GLFW could not initialized!");
            const char* message;
            glfwGetError(&message);
            FOO_ENGINE_ERROR(message);
            return;
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_WindowHandle =
            glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(m_WindowHandle, &m_Data);
        glfwSetErrorCallback(GLFWErrorCallback);
        if (!glfwVulkanSupported())
        {
            FOO_ENGINE_CRITICAL("Vulkan not supported!");
            return;
        }

        glfwSetWindowSizeCallback(m_WindowHandle,
                                  [](GLFWwindow* window, int w, int h)
                                  {
                                      WindowData& data =
                                          *(WindowData*)glfwGetWindowUserPointer(window);
                                      data.Width  = w;
                                      data.Height = h;
                                      WindowResizeEvent e(w, h);
                                      data.eventCallback(e);
                                  });
        glfwSetWindowCloseCallback(m_WindowHandle,
                                   [](GLFWwindow* window)
                                   {
                                       WindowData& data =
                                           *(WindowData*)glfwGetWindowUserPointer(window);
                                       WindowCloseEvent e;
                                       data.eventCallback(e);
                                   });
        glfwSetKeyCallback(m_WindowHandle,
                           [](GLFWwindow* window, int key, int scancode, int action, int mods)
                           {
                               WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                               switch (action)
                               {
                                   case GLFW_PRESS:
                                   {
                                       KeyPressedEvent event(key, 0);
                                       data.eventCallback(event);
                                       break;
                                   }
                                   case GLFW_RELEASE:
                                   {
                                       KeyReleasedEvent event(key);
                                       data.eventCallback(event);
                                       break;
                                   }
                                   case GLFW_REPEAT:
                                   {
                                       KeyPressedEvent event(key, true);
                                       data.eventCallback(event);
                                       break;
                                   }
                               }
                           });
        glfwSetCharCallback(m_WindowHandle,
                            [](GLFWwindow* window, unsigned int keycode)
                            {
                                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                                KeyTypedEvent e(keycode);
                                data.eventCallback(e);
                            });

        glfwSetFramebufferSizeCallback(m_WindowHandle,
                                       [](GLFWwindow* window, int width, int height)
                                       {
                                           WindowData& data =
                                               *(WindowData*)glfwGetWindowUserPointer(window);
                                           data.Width  = width;
                                           data.Height = height;
                                           WindowResizeEvent e{static_cast<unsigned int>(width),
                                                               static_cast<unsigned int>(height)};
                                           data.eventCallback(e);
                                       });
        glfwSetMouseButtonCallback(
            m_WindowHandle,
            [](GLFWwindow* window, int button, int action, int mods)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                switch (action)
                {
                    case GLFW_PRESS:
                    {
                        MouseButtonPressedEvent e{static_cast<MouseCode>(button)};
                        data.eventCallback(e);
                        break;
                    }
                    case GLFW_RELEASE:
                    {
                        MouseButtonReleasedEvent e{static_cast<MouseCode>(button)};
                        data.eventCallback(e);
                        break;
                    }
                }
            });
        glfwSetCursorPosCallback(
            m_WindowHandle,
            [](GLFWwindow* window, double xPos, double yPos)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                data.CursorPosX  = xPos;
                data.CursorPosY  = yPos;

                MouseMovedEvent e{static_cast<float>(xPos), static_cast<float>(yPos)};
                data.eventCallback(e);
            });
        glfwSetScrollCallback(m_WindowHandle,
                              [](GLFWwindow* window, double xOffset, double yOffset)
                              {
                                  WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                                  MouseScrolledEvent e{static_cast<float>(xOffset),
                                                       static_cast<float>(yOffset)};
                                  data.eventCallback(e);
                              });
    }
    void Window::PollEvents()
    {
        glfwPollEvents();
    }
    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(m_WindowHandle);
    }
    void Window::SetCursorCenter()
    {
        glfwSetCursorPos(m_WindowHandle, m_Data.Width / 2, m_Data.Height / 2);
    }
    void Window::Shutdown()
    {
        glfwDestroyWindow(m_WindowHandle);
        glfwTerminate();
    }
    void Window::Close()
    {
        glfwSetWindowShouldClose(m_WindowHandle, true);
    }
    Window::~Window()
    {
        Shutdown();
    }

}  // namespace FooGame
