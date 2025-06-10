#include <GLFW/glfw3.h>
#include "Input.h"
#include "../Core/Application.h"
namespace FooGame {

bool Input::IsKeyReleased(KeyCode keycode) {
  GLFWwindow* windowHandle = Application::Get().GetWindow();
  int state = glfwGetKey(windowHandle, (int)keycode);
  return state == GLFW_RELEASE;
}
bool Input::IsKeyDown(KeyCode keycode) {
  GLFWwindow* windowHandle = Application::Get().GetWindow();
  int state = glfwGetKey(windowHandle, (int)keycode);
  return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonDown(MouseCode keycode) {
  GLFWwindow* windowHandle = Application::Get().GetWindow();
  int state = glfwGetMouseButton(windowHandle, keycode);
  return state == GLFW_PRESS;
}

glm::vec2 Input::GetMousePosition() {
  GLFWwindow* windowHandle = Application::Get().GetWindow();

  double x, y;
  glfwGetCursorPos(windowHandle, &x, &y);
  return {(float)x, (float)y};
}

// void Input::SetCursorMode(CursorMode mode)
// {
//     GLFWwindow *windowHandle = Window::Get().GetWindowHandle();
//     glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
// }
}  // namespace FooGame
