#include "ScaleYoink.h"
#include "../Core/Time.h"

namespace FooGame
{

    void ScaleYoink::OnCreate()
    {
        m_Transform = &GetComponent<TransformComponent>();
    }
    void ScaleYoink::OnUpdate(float ts)
    {
        m_Transform->Scale.x = sinf(Time::CurrentTime()) * 10;
    }
}  // namespace FooGame
