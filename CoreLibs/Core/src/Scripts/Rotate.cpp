#include "Rotate.h"
#include <cmath>
#include "../Scene/Component.h"
#include "../Engine/Window/Window.h"
namespace FooGame
{

    void RotateScript::OnCreate()
    {
        m_Transform = &GetComponent<TransformComponent>();
    }
    void RotateScript::OnUpdate(float ts)
    {
        double time                = Window::Get().GetTime();
        m_Transform->Translation.x = sin(time);
    }

}  // namespace FooGame
