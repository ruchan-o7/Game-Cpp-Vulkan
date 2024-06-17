#include "EditorLayer.h"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "imgui.h"
#include "src/Scene/Component.h"
#include <Core.h>
#include <cstdint>
#include <memory>
#include <Core.h>
#include <Log.h>
namespace FooGame
{
    static void DrawEntitySpecs(entt::registry& registry)
    {
        auto Float3 = [&](const char* name, glm::vec3& vec)
        {
            float val[3] = {vec.x, vec.y, vec.z};

            ImGui::DragFloat3(name, val, 0.01f, -9000.0f, 9000.0f);
            vec.x = val[0];
            vec.y = val[1];
            vec.z = val[2];
        };

#define REMOVE_COMP_BTN(ent, type)         \
    if (ImGui::Button("Delete component")) \
    {                                      \
        registry.remove<type>(ent);        \
    }

        auto entites = registry.view<TransformComponent>().each();
        ImGui::Begin("Entites");
        int i = 0;
        for (auto [entity, transform] : entites)
        {
            auto tag      = registry.get<TagComponent>(entity);
            auto idComp   = registry.get<IDComponent>(entity);
            auto meshComp = registry.try_get<MeshRendererComponent>(entity);
            auto* sc      = registry.try_get<ScriptComponent>(entity);
            if (ImGui::CollapsingHeader(tag.Tag.c_str()))
            {
                ImGui::PushID(i);
                uint64_t id = idComp.ID;
                ImGui::Text("ID: %llu", id);

                if (ImGui::TreeNode("Transform"))
                {
                    Float3("Translation", transform.Translation);
                    Float3("Scale", transform.Scale);
                    Float3("Rotation", transform.Rotation);
                    ImGui::TreePop();
                }

                if (meshComp)
                {
                    if (ImGui::TreeNode("MeshRenderer"))
                    {
                        ImGui::LabelText(meshComp->ModelName.c_str(), "Model");
                        REMOVE_COMP_BTN(entity, MeshRendererComponent);
                        ImGui::TreePop();
                    }
                }
                if (sc)
                {
                    if (ImGui::TreeNode("Script"))
                    {
                        for (auto& [name, _] : sc->Scripts)
                        {
                            bool shouldRemove = false;
                            if (ImGui::Button("Remove script"))
                            {
                                shouldRemove = true;
                            }
                            ImGui::SameLine();
                            ImGui::TextUnformatted(name.c_str());
                            if (shouldRemove)
                            {
                                sc->RemoveScript(name);
                                break;
                            }
                        }
                        REMOVE_COMP_BTN(entity, ScriptComponent);

                        ImGui::TreePop();
                    }
                }

                if (ImGui::Button("Add component"))
                {
                    ImGui::OpenPopup("ComponentsPopup");
                }
                if (ImGui::BeginPopup("ComponentsPopup"))
                {
                    ImGui::SeparatorText("Components");
                    if (!meshComp)
                    {
                        if (ImGui::Selectable("MESH"))
                        {
                        }
                    }
                    if (!sc)
                    {
                        if (ImGui::Selectable("SCRIPT"))
                        {
                        }
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            i++;
        }
        ImGui::End();
#undef REMOVE_COMP_BTN
    }
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
            Renderer3D::SubmitModel(comp.ModelName);
        }

        m_Camera2.setPerspective(60.0f, (float)1600.0f / (float)900.0f, 0.1f, 512.0f);
        m_Camera2.SetRotation(glm::vec3(-12.0f, 159.0f, 0.0f));
        m_Camera2.SetTranslatin(glm::vec3(0.0f, 0.5f, 0.5f));
        m_Camera2.MovementSpeed = 0.1f;
        m_Camera2.Fov           = 90.f;
        m_Camera2.type          = Camera::CameraType::firstperson;
    }
    void EditorLayer::OnDetach()
    {
        Renderer3D::ClearBuffers();
    }
    void EditorLayer::OnImGuiRender()
    {
        DrawCameraUI();
        DrawEntitySpecs(m_Scene->m_Registry);
    }
    void EditorLayer::DrawCameraUI()
    {
        ImGui::Begin("Camera");
        auto cameraPos = m_Camera2.Position;
        float pos[3]   = {cameraPos.x, cameraPos.y, cameraPos.z};
        float fov      = m_Camera2.Fov;
        float rot[3]   = {
            m_Camera2.Rotation.x,
            m_Camera2.Rotation.y,
            m_Camera2.Rotation.z,
        };
        float front[3] = {
            m_Camera2.Front.x,
            m_Camera2.Front.y,
            m_Camera2.Front.z,
        };

        ImGui::DragFloat("Movement Speed", &m_Camera2.MovementSpeed, 0.1f, 0.1f, 5.0f);
        ImGui::DragFloat("Rotation Speed", &m_Camera2.RotationSpeed, 0.1f, 0.1f, 5.0f);
        ImGui::Checkbox("Flip y", &m_Camera2.flipY);
        ImGui::DragFloat("Fov", &fov, 0.1f, 0.1f, 179.0f);
        ImGui::DragFloat3("Position", pos, 0.1f, -1000.0f, 1000.0f);
        ImGui::DragFloat3("Rotation", rot, 0.1f, -1000.0f, 1000.0f);
        ImGui::DragFloat("ZNear", &m_Camera2.ZNear, 0.01f, 0.0f, 1000000.0f);
        ImGui::DragFloat("ZFar", &m_Camera2.ZFar, 0.01f, 0.0f, 1000000.0f);
        ImGui::End();

        m_Camera2.SetPosition(glm::vec3{pos[0], pos[1], pos[2]});
        m_Camera2.SetRotation(glm::vec3(rot[0], rot[1], rot[2]));
        m_Camera2.SetFov(fov);
    }
    void EditorLayer::OnRender()
    {
        Renderer3D::BeginScene(m_Camera2);
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
        UpdateCamera(ts);
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
            int32_t dx                 = (int32_t)m_Camera2.LastMouseState.x - event.GetX();
            int32_t dy                 = (int32_t)m_Camera2.LastMouseState.y - event.GetY();
            m_Camera2.LastMouseState.x = dx;
            m_Camera2.LastMouseState.y = dy;
            m_Camera2.Rotate(
                glm::vec3(dy * m_Camera2.RotationSpeed, -dx * m_Camera2.RotationSpeed, 0.0f));
        }
        else
        {
            Input::SetCursorMode(CursorMode::Normal);
        }
        return true;
    }
    void EditorLayer::UpdateCamera(float ts)
    {
        if (Input::IsKeyDown(KeyCode::W))
        {
            m_Camera2.MoveUp();
        }
        if (Input::IsKeyDown(KeyCode::S))
        {
            m_Camera2.MoveDown();
        }
        if (Input::IsKeyDown(KeyCode::D))
        {
            m_Camera2.MoveRight();
        }
        if (Input::IsKeyDown(KeyCode::A))
        {
            m_Camera2.MoveLeft();
        }
        m_Camera2.Update(ts);

        if (Input::IsMouseButtonDown(MouseButton::Right))
        {
            const glm::vec2& mouse{Input::GetMousePosition().x, Input::GetMousePosition().y};
            glm::vec2 delta                 = (mouse - m_Camera.m_InitialMousePosition) * 0.003f;
            m_Camera.m_InitialMousePosition = mouse;
            m_Camera.Rotate(delta);
        }
    }
}  // namespace FooGame
