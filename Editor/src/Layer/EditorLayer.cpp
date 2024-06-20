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
        SceneSerializer serializer(m_Scene.get());
        serializer.DeSerialize("Assets/Scenes/Prototype2/Scene.json");

        auto mv = m_Scene->GetAllEntitiesWith<MeshRendererComponent>().each();
        for (auto [entity, comp] : mv)
        {
            // Renderer3D::SubmitModel(comp.ModelName);  // TODO move to asset manager
        }

        auto entt          = m_Scene->GetPrimaryCameraEntity();
        auto& cameraComp   = entt.GetComponent<CameraComponent>();
        cameraComp.pCamera = &m_Camera2;
        m_Panel            = new SceneHierarchyPanel(m_Scene.get());
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
            SceneSerializer serializer{m_Scene.get()};
            serializer.Serialize("Scenes\\Prototype2\\Scene.json");
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
        if (Input::IsMouseButtonDown(MouseButton::Right))
        {
            Input::SetCursorMode(CursorMode::Locked);
        }
        else
        {
            Input::SetCursorMode(CursorMode::Normal);
        }
        return true;
    }
    void EditorLayer::UpdateCamera(float ts)
    {
    }
}  // namespace FooGame
