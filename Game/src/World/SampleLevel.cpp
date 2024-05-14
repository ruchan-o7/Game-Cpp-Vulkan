#include "Level.h"
#include <Core/Graphics/Renderer3D.h>
#include <imgui.h>
namespace FooGame
{
    SampleLevel::SampleLevel()
    {
        OnAttach();
    }
    void SampleLevel::OnAttach()
    {
        std::cout << "Attached" << std::endl;
    }
    void SampleLevel::OnUpdate(float deltaTime)
    {
    }
    void SampleLevel::OnRender()
    {
        Renderer3D::BeginDraw();
        {
            Renderer3D::BeginScene(m_Camera);
            Renderer3D::DrawModel();
            Renderer3D::EndScene();
        }
        Renderer3D::EndDraw();
    }
    void SampleLevel::OnUI()
    {
        auto posisitons = m_Camera.GetPosition();
        float pos[3]    = {posisitons.x, posisitons.y, posisitons.z};
        ImGui::SliderFloat3("Camera pos", pos, -10.0f, 10.0f);
        m_Camera.SetPosition({pos[0], pos[1], pos[2]});
    }

}  // namespace FooGame
