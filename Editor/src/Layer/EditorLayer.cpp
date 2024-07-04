#include "EditorLayer.h"
#include "entt/entt.hpp"
#include <Core.h>
#include <Log.h>
#include <memory>

namespace FooGame
{

    EditorLayer::EditorLayer(const ApplicationCommandLineArgs& args)
        : Layer("Editor Layer"), m_Args(args), m_Scene(std::make_unique<Scene>())
    {
        FOO_EDITOR_INFO("Editor layer Created");
    }
    void EditorLayer::OnAttach()
    {
        FOO_EDITOR_INFO("Reading scene data");
        SceneSerializer serializer("Assets\\Scenes\\Prototype3\\Scene.json", m_Scene.get());
        serializer.DeSerialize();

        auto mv = m_Scene->GetAllEntitiesWith<ModelRendererComponent>().each();

        auto entt = m_Scene->GetPrimaryCameraEntity();
        if (entt)
        {
            auto& cameraComp   = entt.GetComponent<CameraComponent>();
            cameraComp.pCamera = &m_Camera2;
        }
        else
        {
            auto mainCameraEntity = m_Scene->CreateEntity("Main Camera");
            auto& cam             = mainCameraEntity.AddComponent<CameraComponent>();
            cam.pCamera           = &m_Camera2;
            cam.Primary           = true;
            cam.FixedAspectRatio  = true;
            auto& sc              = mainCameraEntity.AddComponent<ScriptComponent>();

            sc.Bind("CameraController");
        }
        m_Panel = new SceneHierarchyPanel(m_Scene.get());
    }
    void EditorLayer::OnDetach()
    {
        Renderer3D::ClearBuffers();
    }
    void EditorLayer::OnImGuiRender()
    {
        m_Panel->OnImgui();
    }
    void EditorLayer::OnUpdate(float ts)
    {
        m_Scene->OnUpdate(ts);
        if (Input::IsKeyDown(Key::F1))
        {
            SceneSerializer serializer{"Assets\\Scenes\\Prototype3\\Scene.json", m_Scene.get()};
            serializer.Serialize();
        }
        m_Scene->RenderScene();
    }
    void EditorLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(EditorLayer::OnMouseMoved));
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(EditorLayer::OnMousePressed));
    }
    bool EditorLayer::OnMousePressed(MouseButtonPressedEvent& event)
    {
        return true;
    }
    bool EditorLayer::OnMouseMoved(MouseMovedEvent& event)
    {
        return true;
    }
}  // namespace FooGame
