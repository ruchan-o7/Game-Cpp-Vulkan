#include "Core/Core/Base.h"
#include "Core/Core/Window.h"
#include "Core/Graphics/Renderer2D.h"
#include "Core/Graphics/Texture2D.h"
#include "Core/Scene/GameObject.h"
#include "Core/Scene/Component.h"
#include "Scene.h"
#include <Core/Graphics/Renderer3D.h>
#include <imgui.h>
#include <cmath>
namespace FooGame
{

#if 1
#define MODEL_PATH    "../../../Assets/Model/viking_room.obj"
#define MODEL_TEXTURE "../../../Assets/Model/viking_room.png"
#else
#define MODEL_PATH    "../../Assets/Model/viking_room.obj"
#define MODEL_TEXTURE "../../Assets/Model/viking_room.png"
#endif
    SampleScene::SampleScene()
    {
        OnAttach();
    }
    void SampleScene::OnAttach()
    {
        std::cout << "Attached" << std::endl;
        {
            Objects["viking 1"] = CreateShared<GameObject>("viking 1");
            Objects["viking 2"] = CreateShared<GameObject>("viking 2");

            auto model1  = Model::LoadModel(MODEL_PATH);
            auto texture = LoadTexture(MODEL_TEXTURE);
            model1->GetMeshes()[0].SetTexture(texture);
            u32 id = Renderer3D::SubmitModel(model1);
            model1->SetId(id);
            auto renderer = CreateShared<MeshRendererComponent>(model1);
            Objects["viking 1"]->AddComponent(renderer);
        }
        {
            auto model2  = Model::LoadModel(MODEL_PATH);
            auto texture = LoadTexture(MODEL_TEXTURE);
            model2->GetMeshes()[0].SetTexture(texture);
            u32 id        = Renderer3D::SubmitModel(model2);
            auto renderer = CreateShared<MeshRendererComponent>(model2);
            Objects["viking 2"]->AddComponent(renderer);
            model2->SetId(id);
        }
    }
    void SampleScene::OnUpdate(float deltaTime)
    {
        Objects["viking 1"]->Transform.Position.x =
            sinf((float)WindowsWindow::Get().GetTime());
        Objects["viking 1"]->Transform.Position.y =
            cosf((float)WindowsWindow::Get().GetTime());
        float width  = WindowsWindow::Get().GetWidth();
        float height = WindowsWindow::Get().GetHeight();
        m_Ortho.SetProj(0.0f, width, 0.0f, height);
        m_Camera.SetAspect(width / height);
    }
    float sizeX = 10.0f, sizeY = 10.0f;
    float x = 20.0f, y = 20.f;
    void SampleScene::OnRender()
    {
        Renderer3D::BeginDraw();
        {
            Renderer3D::BeginScene(m_Camera);
            for (auto& o : Objects)
            {
                if (!o.second->IsEnabled())
                {
                    continue;
                }
                if (auto mrc = o.second->GetComponent<MeshRendererComponent>())
                {
                    Renderer3D::DrawModel(o.second);
                }
            }
            Renderer3D::EndScene();
        }
        Renderer3D::EndDraw();
        Renderer2D::BeginDraw();
        {
            Renderer2D::BeginScene(m_Ortho);
            ImGui::Begin("Quad");
            float pos[2]  = {x, y};
            float size[2] = {sizeX, sizeY};
            ImGui::SliderFloat2("Position", pos, -200.0f, 200.0f);
            ImGui::SliderFloat2("Scale", size, -200.0f, 200.0f);
            x     = pos[0];
            y     = pos[1];
            sizeX = size[0];
            sizeY = size[1];
            Renderer2D::DrawQuad({x, y}, {sizeX, sizeY},
                                 {1.0f, 1.0f, 1.0f, 1.0f});
            ImGui::End();
            Renderer2D::EndScene();
        }
        Renderer2D::EndDraw();
    }
    void SampleScene::OnUI()
    {
        auto posisitons = m_Camera.GetPosition();
        float pos[3]    = {posisitons.x, posisitons.y, posisitons.z};
        ImGui::SliderFloat3("Camera pos", pos, -10.0f, 10.0f);
        m_Camera.SetPosition({pos[0], pos[1], pos[2]});
        ImGui::Begin("Game objects");
        for (auto& obj : Objects)
        {
            auto& o      = obj.second;
            float pos[3] = {o->Transform.Position.x, o->Transform.Position.y,
                            o->Transform.Position.z};
            ImGui::SliderFloat3("Position", pos, -10.f, 10.0f);

            float rot[3] = {o->Transform.Rotation.x, o->Transform.Rotation.y,
                            o->Transform.Rotation.z};
            ImGui::SliderFloat3("Rotation", rot, -10.f, 10.0f);

            float scale[3] = {o->Transform.Scale.x, o->Transform.Scale.y,
                              o->Transform.Scale.z};
            ImGui::SliderFloat3("Scale", scale, 0.f, 10.0f);

            o->Transform.Position = {pos[0], pos[1], pos[2]};
            o->Transform.Rotation = {rot[0], rot[1], rot[2]};
            o->Transform.Scale    = {scale[0], scale[1], scale[2]};
        }

        ImGui::End();
    }

}  // namespace FooGame
