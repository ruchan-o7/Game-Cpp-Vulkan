#include "EditorLayer.h"
#include <Engine.h>
#include <imgui.h>
#include <fstream>
#include "src/Engine/Renderer3D.h"
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
    }
    void EditorLayer::OnAttach()
    {
#ifdef FOO_DEBUG
        std::ifstream f(SCENE_JSON);
        m_EditorScene = EditorScene::LoadScene(f);
        f.close();
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
        for (auto& [t, m, id, tIndex] : m_EditorScene->Meshes)
        {
            vertexSize += m->m_Vertices.size() * sizeof(Vertex);
            indexSize  += m->m_Indices.size() * sizeof(uint32_t);
        }
        std::cout << "Will allocate " << vertexSize
                  << " of bytes for vertices\n";
        std::cout << "Will allocate " << indexSize << " of bytes for indices\n";
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
        ImGui::Text("X: %f, Y: %f, Z: %f", pos[0], pos[1], pos[2]);
        m_Camera.SetPosition({pos[0], pos[1], pos[2]});
        ImGui::End();
    }
    void EditorLayer::OnUpdate(float ts)
    {
        UpdateCamera();

        Renderer3D::BeginDraw();
        Renderer3D::BeginScene(m_Camera);
        for (auto& [transform, mesh, id, tIndex] : m_EditorScene->Meshes)
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
    void EditorLayer::UpdateCamera()
    {
        auto pos = m_Camera.GetPosition();
        if (Input::IsKeyDown(KeyCode::W))
        {
            pos.z += 1.0f;
        }
        if (Input::IsKeyDown(KeyCode::S))
        {
            pos.z -= 1.0f;
        }
        m_Camera.SetPosition(pos);
    }
}  // namespace FooGame
