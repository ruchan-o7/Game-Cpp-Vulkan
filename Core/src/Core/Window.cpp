#include "Window.h"
#include "../Events/KeyEvent.h"
#include "../Events/ApplicationEvent.h"
#include "GLFW/glfw3.h"
#include "../Input/MouseCodes.h"
#include "../Events/MouseEvent.h"
#include "Log.h"
namespace FooGame {
static Window* s_Instance = nullptr;
Window& Window::Get() {
  return *s_Instance;
}
double Window::GetTime() {
  return glfwGetTime();
}
HWND Window::GetWin32NativeHandle() const {
  return glfwGetWin32Window(m_WindowHandle);
}

Window::Window(WindowProperties specifications) {
  s_Instance = this;
  Init(specifications);
}
inline void Window::WaitEvent() {
  glfwWaitEvents();
}

void Window::SetWindowTitle(const char* title) {
  m_Data.Title = title;
  glfwSetWindowTitle(m_WindowHandle, title);
}
double Window::GetCursorPosX() const {
  return m_Data.CursorPosX;
}
double Window::GetCursorPosY() const {
  return m_Data.CursorPosY;
}
void Window::Init(const WindowProperties& props) {
  FOO_ENGINE_TRACE("Window creating");
  m_Data.Height = props.Height;
  m_Data.Width = props.Width;
  m_Data.Title = props.Title;
  if (!glfwInit()) {
    FOO_ENGINE_CRITICAL("GLFW could not initialized!");
    const char* message;
    glfwGetError(&message);
    FOO_ENGINE_ERROR(message);
    return;
  }

  m_WindowHandle =
      glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
}
void Window::PollEvents() {
  glfwPollEvents();
}
bool Window::ShouldClose() {
  return glfwWindowShouldClose(m_WindowHandle);
}
void Window::SetCursorCenter() {
  glfwSetCursorPos(m_WindowHandle, m_Data.Width / 2, m_Data.Height / 2);
}
void Window::Shutdown() {
  glfwDestroyWindow(m_WindowHandle);
  glfwTerminate();
}
void Window::Close() {
  glfwSetWindowShouldClose(m_WindowHandle, true);
}
Window::~Window() {
  Shutdown();
}

}  // namespace FooGame
