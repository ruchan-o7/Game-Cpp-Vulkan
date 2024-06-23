#include "EditorLayer.h"
#include "entt/entt.hpp"
#include "imgui.h"
#include <Core.h>
#include <Log.h>
#include <memory>

namespace FooGame
{

    EditorLayer::EditorLayer(const CommandLineArgs& args)
        : Layer("Editor Layer"), m_Args(args), m_Scene(std::make_unique<Scene>())
    {
        FOO_EDITOR_INFO("Editor layer Created");
    }
    void EditorLayer::OnAttach()
    {
        FOO_EDITOR_INFO("Reading scene data");
        SceneSerializer serializer("Assets\\Scenes\\Prototype3\\Scene.json", m_Scene.get());
        serializer.DeSerialize();

        auto mv = m_Scene->GetAllEntitiesWith<MeshRendererComponent>().each();

        auto entt = m_Scene->GetPrimaryCameraEntity();
        if (entt)
        {
            auto& cameraComp   = entt.GetComponent<CameraComponent>();
            cameraComp.pCamera = &m_Camera2;
        }
        m_Panel = new SceneHierarchyPanel(m_Scene.get());
    }
    void EditorLayer::OnDetach()
    {
        Renderer3D::ClearBuffers();
    }
    void EditorLayer::OnImGuiRender()
    {
        DrawCameraUI();
        m_Panel->OnImgui();
    }
    void EditorLayer::DrawCameraUI()
    {
    }
    void EditorLayer::OnRender()
    {
        m_Scene->RenderScene();
        auto stats = Renderer3D::GetStats();
        ImGui::Begin("3d scene stats");
        ImGui::Text("Draw calls %i", stats.DrawCall);
        ImGui::Text("Vertex count %llu", stats.VertexCount);
        ImGui::Text("Index count %llu", stats.IndexCount);
        ImGui::End();
        Renderer3D::EndDraw();
    }
    void EditorLayer::OnUpdate(float ts)
    {
        m_Scene->OnUpdate(ts);
        if (Input::IsKeyDown(KeyCode::F1))
        {
            SceneSerializer serializer{"Assets\\Scenes\\Prototype3\\Scene.json", m_Scene.get()};
            serializer.Serialize();
        }
    }
    void EditorLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(EditorLayer::OnMouseMoved));
    }
    bool EditorLayer::OnMousePressed(MouseButtonPressedEvent& event)
    {
        return true;
    }
    bool EditorLayer::OnMouseMoved(MouseMovedEvent& event)
    {
        return true;
    }
    void EditorLayer::UpdateCamera(float ts)
    {
    }
}  // namespace FooGame
