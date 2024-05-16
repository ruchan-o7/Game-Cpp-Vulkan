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
        // Load Scene model to renderer
        std::cout << "Attached" << std::endl;
        Objects["viking 1"] = CreateShared<GameObject>("viking 1");
        Objects["viking 2"] = CreateShared<GameObject>("viking 2");
        auto texture        = LoadTexture(MODEL_TEXTURE);
        {
            auto model1 = Model::LoadModel(MODEL_PATH);
            model1->GetMeshes()[0].SetTexture(texture);
            Renderer3D::SubmitModel(model1);
            auto renderer = CreateShared<MeshRendererComponent>(model1);
            Objects["viking 1"]->AddComponent(renderer);
        }
        {
            auto model2 = Model::LoadModel(MODEL_PATH);
            model2->GetMeshes()[0].SetTexture(texture);
            Renderer3D::SubmitModel(model2);
            auto renderer = CreateShared<MeshRendererComponent>(model2);
            Objects["viking 2"]->AddComponent(renderer);
            // Execute each gameobjects's OnStart()
        }
    }
    void SampleScene::OnUpdate(float deltaTime)
    {
        float width  = WindowsWindow::Get().GetWidth();
        float height = WindowsWindow::Get().GetHeight();
        m_Ortho.SetProj(0.0f, width, 0.0f, height);
        m_Camera.SetAspect(width / height);
    }
    float quadSizeX = 10.0f, quadSizeY = 10.0f;
    float quadXPos = 20.0f, quadYPos = 20.f;
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
            float pos[2]  = {quadXPos, quadYPos};
            float size[2] = {quadSizeX, quadSizeY};
            ImGui::SliderFloat2("Position", pos, -200.0f, 2000.0f);
            ImGui::SliderFloat2("Scale", size, 0.0f, 1000.0f);
            quadXPos  = pos[0];
            quadYPos  = pos[1];
            quadSizeX = size[0];
            quadSizeY = size[1];
            Renderer2D::DrawQuad({quadXPos, quadYPos}, {quadSizeX, quadSizeY},
                                 {1.0f, 1.0f, 1.0f, 1.0f});
            ImGui::End();
            Renderer2D::EndScene();
        }
        Renderer2D::EndDraw();
    }
    void SampleScene::OnUI()
    {
        auto cameraPos  = m_Camera.GetPosition();
        float camPos[3] = {cameraPos.x, cameraPos.y, cameraPos.z};
        ImGui::SliderFloat3("Camera position", camPos, -10.0f, 10.0f);
        m_Camera.SetPosition({camPos[0], camPos[1], camPos[2]});
        ImGui::Begin("Game objects");
        {
            auto& obj       = Objects["viking 1"];
            float objPos[3] = {obj->Transform.Position.x,
                               obj->Transform.Position.y,
                               obj->Transform.Position.z};
            ImGui::SliderFloat3("Position", objPos, -10.f, 10.0f);

            float objRot[3] = {obj->Transform.Rotation.x,
                               obj->Transform.Rotation.y,
                               obj->Transform.Rotation.z};
            ImGui::SliderFloat3("Rotation", objRot, -10.f, 10.0f);

            float objScale[3] = {obj->Transform.Scale.x, obj->Transform.Scale.y,
                                 obj->Transform.Scale.z};
            ImGui::SliderFloat3("Scale", objScale, 0.f, 10.0f);

            obj->Transform.Position = {objPos[0], objPos[1], objPos[2]};
            obj->Transform.Rotation = {objRot[0], objRot[1], objRot[2]};
            obj->Transform.Scale    = {objScale[0], objScale[1], objScale[2]};
        }
        ImGui::End();
        ImGui::Begin("Game objects 2");
        {
            auto& obj2       = Objects["viking 2"];
            float objPos2[3] = {obj2->Transform.Position.x,
                                obj2->Transform.Position.y,
                                obj2->Transform.Position.z};
            ImGui::SliderFloat3("Position", objPos2, -10.f, 10.0f);

            float objRot2[3] = {obj2->Transform.Rotation.x,
                                obj2->Transform.Rotation.y,
                                obj2->Transform.Rotation.z};
            ImGui::SliderFloat3("Rotation", objRot2, -10.f, 10.0f);

            float objScale2[3] = {obj2->Transform.Scale.x,
                                  obj2->Transform.Scale.y,
                                  obj2->Transform.Scale.z};
            ImGui::SliderFloat3("Scale", objScale2, 0.f, 10.0f);

            obj2->Transform.Position = {objPos2[0], objPos2[1], objPos2[2]};
            obj2->Transform.Rotation = {objRot2[0], objRot2[1], objRot2[2]};
            obj2->Transform.Scale = {objScale2[0], objScale2[1], objScale2[2]};
        }
        ImGui::End();
    }

}  // namespace FooGame
