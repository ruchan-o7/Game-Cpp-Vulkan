#include "Level.h"
#include <Core/Graphics/Renderer3D.h>
#include <imgui.h>
namespace FooGame
{
#define MODEL_PATH "../../../Assets/Model/viking_room.obj"
    SampleLevel::SampleLevel()
    {
        OnAttach();
    }
    void SampleLevel::OnAttach()
    {
        std::cout << "Attached" << std::endl;
        m_Model = Model::LoadModel(MODEL_PATH);
        Renderer3D::SubmitModel(m_Model);
    }
    void SampleLevel::OnUpdate(float deltaTime)
    {
    }
    void SampleLevel::OnRender()
    {
        Renderer3D::BeginDraw();
        {
            Renderer3D::BeginScene(m_Camera);
            Renderer3D::DrawModel(m_Model);
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
