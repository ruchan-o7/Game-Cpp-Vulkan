#include "Input.h"
#include <GLFW/glfw3.h>
#include "../Window.h"
namespace FooGame
{

    bool Input::IsKeyDown(KeyCode keycode)
    {
        GLFWwindow *windowHandle = WindosWindow::Get().GetWindowHandle();
        int state                = glfwGetKey(windowHandle, (int)keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonDown(MouseButton button)
    {
        GLFWwindow *windowHandle = WindosWindow::Get().GetWindowHandle();
        int state = glfwGetMouseButton(windowHandle, (int)button);
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::GetMousePosition()
    {
        GLFWwindow *windowHandle = WindosWindow::Get().GetWindowHandle();

        double x, y;
        glfwGetCursorPos(windowHandle, &x, &y);
        return {(float)x, (float)y};
    }

    void Input::SetCursorMode(CursorMode mode)
    {
        GLFWwindow *windowHandle = WindosWindow::Get().GetWindowHandle();
        glfwSetInputMode(windowHandle, GLFW_CURSOR,
                         GLFW_CURSOR_NORMAL + (int)mode);
    }
}  // namespace FooGame
