#include "Core/Core/Window.h"
#include "Core/Graphics/Texture2D.h"
#include "Level.h"
#include <Core/Graphics/Renderer3D.h>
#include <imgui.h>
#include <cmath>
namespace FooGame
{

#if 0
#define MODEL_PATH    "../../../Assets/Model/viking_room.obj"
#define MODEL_TEXTURE "../../../Assets/Model/viking_room.png"
#else
#define MODEL_PATH    "../../Assets/Model/viking_room.obj"
#define MODEL_TEXTURE "../../Assets/Model/viking_room.png"
#endif
    SampleLevel::SampleLevel()
    {
        OnAttach();
    }
    void SampleLevel::OnAttach()
    {
        std::cout << "Attached" << std::endl;
        {
            m_Model      = Model::LoadModel(MODEL_PATH);
            auto texture = LoadTexture(MODEL_TEXTURE);
            m_Model->GetMeshes()[0].SetTexture(texture);
            u32 id = Renderer3D::SubmitModel(m_Model);
            m_Model->SetId(id);
        }
        {
            m_Model2     = Model::LoadModel(MODEL_PATH);
            auto texture = LoadTexture(MODEL_TEXTURE);
            m_Model2->GetMeshes()[0].SetTexture(texture);
            u32 id = Renderer3D::SubmitModel(m_Model2);
            m_Model2->SetId(id);
        }
    }
    void SampleLevel::OnUpdate(float deltaTime)
    {
        m_Model->Position.x  = sinf((float)WindowsWindow::Get().GetTime());
        m_Model->Position.y  = cosf((float)WindowsWindow::Get().GetTime());
        m_Model2->Position.x = cosf((float)WindowsWindow::Get().GetTime());
        m_Model2->Position.y = sinf((float)WindowsWindow::Get().GetTime());
    }
    void SampleLevel::OnRender()
    {
        Renderer3D::BeginDraw();
        {
            Renderer3D::BeginScene(m_Camera);
            Renderer3D::DrawModel(m_Model);
            Renderer3D::DrawModel(m_Model2);
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
