#include <vector>
#include <Engine.h>
#include "EditorLayer.h"
#include <imgui.h>
#include "EditorSceneDeserializer.h"
#include <Log.h>
namespace FooGame
{

#if 1
#define SCENE_JSON "../../../Assets/Scenes/scene.json"
#else
#define SCENE_JSON "../../Assets/Scenes/scene.json"
#endif
    EditorLayer::EditorLayer(const CommandLineArgs& args)
        : Layer("Editor Layer"), m_Args(args)
    {
        FOO_EDITOR_INFO("Editor layer Created");
    }
    void EditorLayer::OnAttach()
    {
        FOO_EDITOR_INFO("Reading scene data");
#ifdef FOO_DEBUG
        EditorSceneDeserializer serializer;
        m_EditorScene = std::move(serializer.DeSerialize(SCENE_JSON));
#else
        if (m_Args.count > 1)
        {
            std::cout << m_Args.argv[1] << std::endl;
            std::ifstream f{m_Args.argv[1]};
            json data = json::parse(f);
            f.close();
        }

#endif
        size_t vertexSize = 0;
        size_t indexSize  = 0;
        FOO_EDITOR_INFO("Scene : {0}", m_EditorScene->Name);
        FOO_EDITOR_INFO("Textures size : {0}", m_EditorScene->Textures.size());
        FOO_EDITOR_INFO("Mesh size : {0}", m_EditorScene->Meshes.size());
        for (int i = 0; i < m_EditorScene->Meshes.size(); i++)
        {
            auto& mData = m_EditorScene->Meshes[i];

            vertexSize += mData.MeshPtr->m_Vertices.size() * sizeof(Vertex);
            indexSize  += mData.MeshPtr->m_Indices.size() * sizeof(uint32_t);
        }
        FOO_EDITOR_INFO("Will allocate {0} of bytes for vertices", vertexSize);
        FOO_EDITOR_INFO("Will allocate {0} of bytes for indices", indexSize);

        for (auto& [t, m, id, tIndex] : m_EditorScene->Meshes)
        {
            id = Renderer3D::SubmitMesh(m.get());
        }
    }
    void EditorLayer::OnDetach()
    {
        Renderer3D::ClearBuffers();
    }
    void EditorLayer::OnImGuiRender()
    {
        ImGui::Begin("Camera pos");
        auto cameraPos = m_Camera.GetPosition();
        float pos[3]   = {cameraPos.x, cameraPos.y, cameraPos.z};
        ImGui::SliderFloat3("Position", pos, -20.0f, 20.0f);
        m_Camera.SetPosition(glm::vec3{pos[0], pos[1], pos[2]});
        ImGui::DragFloat("Yaw", &m_Camera.m_Yaw, 0.5f, -180.f, 180.f);
        ImGui::DragFloat("Pitch", &m_Camera.m_Pitch, 0.5f, -180.f, 180.f);
        ImGui::DragFloat("Near clip", &m_Camera.m_NearClip, 0.5f, 0.0001f,
                         10.f);
        ImGui::DragFloat("Far clip", &m_Camera.m_FarClip, 0.1f, 1.0f, 10000.0f);
        ImGui::DragFloat("Aspect ", &m_Camera.m_Aspect, 0.1f, 0.001f, 3.0f);
        ImGui::DragFloat("Zoom ", &m_Camera.m_Zoom, 0.1f, 0.1f, 179.0f);
        float dir[3] = {
            m_Camera.m_Direction.x,
            m_Camera.m_Direction.y,
            m_Camera.m_Direction.z,
        };
        ImGui::DragFloat3("Direction ", dir, 0.1f);
        float up[3] = {
            m_Camera.m_Up.x,
            m_Camera.m_Up.y,
            m_Camera.m_Up.z,
        };
        ImGui::DragFloat3("Up ", up, 0.1f);
        m_Camera.m_Up.x        = up[0];
        m_Camera.m_Up.y        = up[1];
        m_Camera.m_Up.z        = up[2];
        m_Camera.m_Direction.x = dir[0];
        m_Camera.m_Direction.y = dir[1];
        m_Camera.m_Direction.z = dir[2];

        ImGui::End();
        int i = 0;
        for (auto& m : m_EditorScene->Meshes)
        {
            float pos[3] = {
                m.Transform.Translation.x,
                m.Transform.Translation.y,
                m.Transform.Translation.z,
            };
            float scale[3] = {
                m.Transform.Scale.x,
                m.Transform.Scale.y,
                m.Transform.Scale.z,
            };
            float rot[3] = {
                m.Transform.Rotation.x,
                m.Transform.Rotation.y,
                m.Transform.Rotation.z,
            };
            ImGui::Begin("mesh");
            ImGui::PushID(i);

            ImGui::SliderFloat3("Position", pos, -99.f, 99.f);
            ImGui::SliderFloat3("Scale", scale, 0.1f, 99.f);
            ImGui::SliderFloat3("Rotation", rot, -99.f, 99.f);
            ImGui::PopID();

            ImGui::End();
            m.Transform.Translation = glm::vec3{
                pos[0],
                pos[1],
                pos[2],
            };
            m.Transform.Scale = glm::vec3{
                scale[0],
                scale[1],
                scale[2],
            };
            m.Transform.Rotation = glm::vec3{
                rot[0],
                rot[1],
                rot[2],
            };
            i++;
        }
    }
    void EditorLayer::OnUpdate(float ts)
    {
        UpdateCamera(ts);

        Renderer3D::BeginDraw();
        Renderer3D::BeginScene(m_Camera);
        for (auto [transform, mesh, id, tIndex] : m_EditorScene->Meshes)
        {
            auto texture = m_EditorScene->Textures[tIndex];
            Renderer3D::DrawMesh(id, transform.GetTransform(), *texture);
        }
        Renderer3D::EndScene();
        Renderer3D::EndDraw();
    }
    void EditorLayer::OnEvent(Event& e)
    {
    }
    static float Sensitivity = 0.1f;
    void EditorLayer::UpdateCamera(float ts)
    {
        auto pos = m_Camera.GetPosition();
        if (Input::IsKeyDown(KeyCode::W))
        {
            pos.z += 1.0f * Sensitivity * ts;
        }
        if (Input::IsKeyDown(KeyCode::S))
        {
            pos.z -= 1.0f * Sensitivity * ts;
        }
        if (Input::IsMouseButtonDown(MouseButton::Right))
        {
            const glm::vec2& mouse{Input::GetMousePosition().x,
                                   Input::GetMousePosition().y};
            glm::vec2 delta =
                (mouse - m_Camera.m_InitialMousePosition) * 0.003f;
            m_Camera.m_InitialMousePosition = mouse;
            m_Camera.Rotate(delta);
        }
        m_Camera.SetPosition(pos);
        m_Camera.SetAspect((float)WindowsWindow::Get().GetWidth() /
                           (float)WindowsWindow::Get().GetHeight());
    }
}  // namespace FooGame
