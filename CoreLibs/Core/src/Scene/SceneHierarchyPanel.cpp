#include "SceneHierarchyPanel.h"
#include "../Core/AssetManager.h"
#include "../Engine/Geometry/Model.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <string.h>
#include <filesystem>
#include <unordered_map>
#include "Entity.h"
#include "Scene.h"
#include "Component.h"
#include "../Scripts/CameraController.h"
#include "../Scripts/Rotate.h"
#include "../Scripts/ScaleYoink.h"
#include "../Base.h"
#include "../Core/File.h"
#include "../Core/ObjLoader.h"
#include "../Base.h"
#include "src/Engine/Geometry/Material.h"
namespace FooGame
{
    static const std::string scriptNames[] = {"RotateScript", "ScaleYoink", "CameraController"};
    SceneHierarchyPanel::SceneHierarchyPanel(Scene* context)
    {
        SetContext(context);
    }
    void SceneHierarchyPanel::SetContext(Scene* context)
    {
        m_pScene = context;
    }
    void SceneHierarchyPanel::DrawMaterialSection()
    {
        ImGui::Begin("Scene materials");
        {
            auto& materials = AssetManager::GetAllMaterials();
            auto avail      = ImGui::GetContentRegionAvail();
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            Defer pc{[&] { ImGui::PopStyleVar(); }};
            ImGui::BeginChild("material_list", ImVec2(avail.x, 200), ImGuiChildFlags_Border,
                              ImGuiWindowFlags_MenuBar);
            Defer cd{[&] { ImGui::EndChild(); }};
            {
                ImGui::BeginMenuBar();

                Defer mbd{[&] { ImGui::EndMenuBar(); }};

                if (ImGui::BeginMenu("File"))
                {
                    Defer mc{[&] { ImGui::EndMenu(); }};
                    if (ImGui::MenuItem("Add Material"))
                    {
                        Material newMat{};
                        newMat.Name                        = "New Material";
                        newMat.PbrMat.BaseColorTextureName = DEFAULT_TEXTURE_NAME;
                        AssetManager::AddMaterial(newMat);
                    }
                    if (ImGui::MenuItem("Add Texture"))
                    {
                        List<std::filesystem::path> outPaths;
                        File::OpenFileDialog(outPaths);
                        for (auto& p : outPaths)
                        {
                            auto extension = p.extension().string();
                            if (extension != ".png" || extension != ".jpg" || extension != ".jpeg")
                            {
                                AssetManager::LoadTexture(p.string(), p.filename().string());
                            }
                        }
                    }
                }
            }
            String willDeleteMat;
            for (auto& [name, mat] : materials)
            {
                if (ImGui::Selectable(name.c_str()))
                {
                    m_SelectedMaterial = name.c_str();
                }
                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::Button("Remove material"))
                    {
                        willDeleteMat = name;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            if (!willDeleteMat.empty())
            {
                materials.extract(willDeleteMat);
            }
        }
        DrawMaterial();
        ImGui::End();
    }

    void SceneHierarchyPanel::OnImgui()
    {
        ImGui::Begin("Scene hierarchy");
        if (m_pScene)
        {
            m_pScene->m_Registry.view<TransformComponent>().each(
                [&](auto entityID, auto transform)
                {
                    Entity entity{entityID, m_pScene};
                    DrawEntityNode(entity);
                });
            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            {
                m_SelectionContext = {};
                m_SelectedMaterial = nullptr;
            }
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                {
                    auto e = m_pScene->CreateEntity("Empty entity");
                }
                ImGui::EndPopup();
            }
        }
        ImGui::Begin("Properties");
        if (m_SelectionContext)
        {
            DrawComponents(m_SelectionContext);
        }

        DrawMaterialSection();

        ImGui::End();

        ImGui::End();
    }
    void SceneHierarchyPanel::DrawMaterial()
    {
        auto avail = ImGui::GetContentRegionAvail();
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        Defer pc{[&] { ImGui::PopStyleVar(); }};
        ImGui::BeginChild("material_props", avail, ImGuiChildFlags_Border,
                          ImGuiWindowFlags_MenuBar);
        Defer cd{[&] { ImGui::EndChild(); }};
        if (m_SelectedMaterial)
        {
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            ImGui::SeparatorText("Material Properties");
            auto* material = AssetManager::GetMaterial(m_SelectedMaterial);
            strncpy_s(buffer, sizeof(buffer), material->Name.c_str(), sizeof(buffer));
            if (material->Name != DEFAULT_MATERIAL_NAME)
            {
                if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
                {
                    auto& allMaterials = AssetManager::GetAllMaterials();
                    Material newMaterial;
                    newMaterial.Name                        = material->Name;
                    newMaterial.PbrMat.BaseColorTextureName = material->PbrMat.BaseColorTextureName;
                    allMaterials.extract(material->Name);
                    newMaterial.Name = std::string(buffer);
                    AssetManager::AddMaterial(newMaterial);
                    material = AssetManager::GetMaterial(m_SelectedMaterial);

                    m_SelectedMaterial = material->Name.c_str();
                }
            }
            else
            {
                ImGui::Text("%s", material->Name.c_str());
            }
            ImGui::Text("Base color: %s", material->PbrMat.BaseColorTextureName.c_str());
            ImGui::SameLine();
            if (ImGui::Button("Change"))
            {
                ImGui::OpenPopup("texture_popup_base");
            }
            if (ImGui::BeginPopup("texture_popup_base"))
            {
                ImGui::SeparatorText("All Textures");
                auto textures = AssetManager::GetAllImages();
                for (auto& [name, tex] : textures)
                {
                    if (ImGui::Selectable(name.c_str()))
                    {
                        material->PbrMat.BaseColorTextureName = String(name);
                    }
                }
                ImGui::EndPopup();
            }
        }
    }
    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags =
            ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
            ImGuiTreeNodeFlags_OpenOnArrow;
        flags       |= ImGuiTreeNodeFlags_SpanAvailWidth;
        bool opened  = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
        if (ImGui::IsItemClicked())
        {
            m_SelectionContext = entity;
        }

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
            {
                entityDeleted = true;
            }

            ImGui::EndPopup();
        }

        if (opened)
        {
            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            bool opened = ImGui::TreeNodeEx((void*)9817239, flags, "%s", tag.c_str());
            if (opened)
            {
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            m_pScene->DestroyEntity(entity);
            if (m_SelectionContext == entity)
            {
                m_SelectionContext = {};
            }
        }
    }
    template <typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
    {
        const ImGuiTreeNodeFlags treeNodeFlags =
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
            ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
            ImGuiTreeNodeFlags_FramePadding;
        if (!entity.HasComponent<T>())
        {
            auto& component               = entity.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open =
                ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.c_str());
            ImGui::PopStyleVar();
            ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
            if (ImGui::Button("+", ImVec2{lineHeight, lineHeight}))
            {
                ImGui::OpenPopup("ComponentSettings");
            }

            bool removeComponent = false;
            if (ImGui::BeginPopup("ComponentSettings"))
            {
                if (ImGui::MenuItem("Remove component"))
                {
                    removeComponent = true;
                }

                ImGui::EndPopup();
            }

            if (open)
            {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (removeComponent)
            {
                entity.RemoveComponent<T>();
            }
        }
    }
    static void DrawVec3Control(const std::string& label, glm::vec3& values,
                                float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGuiIO& io   = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

        float lineHeight  = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
        {
            values.x = resetValue;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
        {
            values.y = resetValue;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
        {
            values.z = resetValue;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();
    }
    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (!entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                tag = std::string(buffer);
            }
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        if (ImGui::Button("Add Component"))
        {
            ImGui::OpenPopup("AddComponent");
        }

        if (ImGui::BeginPopup("AddComponent"))
        {
            DisplayAddComponentEntry<CameraComponent>("Camera");
            DisplayAddComponentEntry<ScriptComponent>("Script");
            DisplayAddComponentEntry<MeshRendererComponent>("Mesh Renderer");

            ImGui::EndPopup();
        }

        ImGui::PopItemWidth();

        DrawComponent<TransformComponent>("Transform", entity,
                                          [](auto& component)
                                          {
                                              DrawVec3Control("Translation", component.Translation);
                                              glm::vec3 rotation = glm::degrees(component.Rotation);
                                              DrawVec3Control("Rotation", rotation);
                                              component.Rotation = glm::radians(rotation);
                                              DrawVec3Control("Scale", component.Scale, 1.0f);
                                          });
        DrawComponent<CameraComponent>("Camera", entity,
                                       [](auto& component)
                                       {
                                           Camera* camera = component.pCamera;

                                           ImGui::Checkbox("Primary", &component.Primary);
                                       });
        DrawComponent<ScriptComponent>(
            "Scripts", entity,
            [](ScriptComponent& component)
            {
                for (auto& [sn, _] : component.Scripts)
                {
                    ImGui::TextUnformatted(sn.c_str());
                    ImGui::SameLine();
                    bool shouldRemove = false;
                    if (ImGui::Button("X"))
                    {
                        shouldRemove = true;
                    }
                    if (shouldRemove)
                    {
                        component.Scripts.erase(sn);
                        break;
                    }
                }
                if (ImGui::Button("Add script"))
                {
                    std::vector<std::string> scripts;
                    int i = 0;
                    for (auto& [name, _] : component.Scripts)
                    {
                        if (auto s = component.Scripts.find(scriptNames[i]);
                            s == component.Scripts.end())
                        {
                            scripts.push_back(scriptNames[i]);
                        }
                        i++;
                    }
                    ImGui::OpenPopup("Add script");
                }
                if (ImGui::BeginPopup("Add script"))
                {
                    if (ImGui::MenuItem(scriptNames[0].c_str()))
                    {
                        component.Bind<RotateScript>(scriptNames[0]);
                        ImGui::CloseCurrentPopup();
                    }
                    else if (ImGui::MenuItem(scriptNames[1].c_str()))
                    {
                        component.Bind<ScaleYoink>(scriptNames[1]);
                        ImGui::CloseCurrentPopup();
                    }
                    else if (ImGui::MenuItem(scriptNames[2].c_str()))
                    {
                        component.Bind<CameraController>(scriptNames[2]);

                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            });
        DrawComponent<MeshRendererComponent>(
            "Mesh Renderer", entity,
            [](MeshRendererComponent& component)
            {
                ImGui::Text("Model: %s", component.ModelName.c_str());
                auto model = AssetManager::GetModel(component.ModelName);
                if (!model)
                {
                    if (ImGui::BeginPopupContextWindow())
                    {
                        if (ImGui::MenuItem("Load Obj"))
                        {
                            List<std::filesystem::path> objs;
                            File::OpenFileDialog(objs);
                            bool isOk = false;
                            for (auto& o : objs)
                            {
                                if (o.extension() != ".obj")
                                {
                                    File::OpenMessageBox("Selected item is not obj");
                                    isOk = false;
                                    break;
                                }
                                isOk = true;
                            }
                            if (!isOk)
                            {
                                return;
                            }
                            for (auto& o : objs)
                            {
                                auto fileNameStr    = o.filename().string();
                                component.ModelName = fileNameStr;
                                ObjLoader loader{o};
                                auto objModel = loader.LoadModel();
                                if (objModel)
                                {
                                    AssetManager::LoadObjModel(std::move(objModel));
                                }
                            }
                        }
                        if (ImGui::MenuItem("Load Gltf"))
                        {
                        }
                        ImGui::EndPopup();
                    }
                    return;
                }

                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                Defer d_1{[&] { ImGui::PopStyleVar(); }};
                ImGui::BeginChild("model_meshes", ImGui::GetContentRegionAvail(),
                                  ImGuiChildFlags_Border);
                Defer d_2{[&] { ImGui::EndChild(); }};

                if (ImGui::BeginTable("materials_", 4))
                {
                    Defer td{[&] { ImGui::EndTable(); }};
                    for (size_t i = 0; i < model->m_Meshes.size(); i++)
                    {
                        ImGui::PushID(i);
                        ImGui::TableNextColumn();
                        ImGui::Text("%zu: ", i);
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", model->m_Meshes[i].Name.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", model->m_Meshes[i].M3Name.c_str());
                        ImGui::TableNextColumn();
                        if (ImGui::Button("Change Material"))
                        {
                            ImGui::OpenPopup("change_mat_pop");
                        }
                        if (ImGui::BeginPopup("change_mat_pop"))
                        {
                            Defer dd_{[&] { ImGui::EndPopup(); }};
                            ImGui::SeparatorText("All Materials");
                            auto materials = AssetManager::GetAllMaterials();
                            for (auto& [name, mat] : materials)
                            {
                                if (ImGui::Selectable(name.c_str()))
                                {
                                    model->m_Meshes[i].M3Name = name;
                                }
                            }
                        }

                        // if (ImGui::Selectable(meshNames[i]))
                        // {
                        //     if (ImGui::Button("Change Material"))
                        //     {
                        //         ImGui::OpenPopup("mesh_change_mat_popup");
                        //     }
                        //     ImGui::SameLine();
                        //     if (ImGui::BeginPopup("mesh_change_mat_popup"))
                        //     {
                        //         Defer d_3{[&] { ImGui::EndPopup(); }};
                        //         ImGui::SeparatorText("All Materials");
                        //         auto materials = AssetManager::GetAllMaterials();
                        //         for (auto& [name, mat] : materials)
                        //         {
                        //             if (ImGui::Selectable(name.c_str()))
                        //             {
                        //                 model->m_Meshes[i].M3Name = name;
                        //             }
                        //         }
                        //     }
                        // }
                        ImGui::PopID();
                    }
                }
                // ImGui::PopItemWidth();
            });
    }
    void SceneHierarchyPanel::SetSelectedEntity(Entity e)
    {
        m_SelectionContext = e;
    }
    Entity SceneHierarchyPanel::GetSelectedEntity()
    {
        return {};
    }
    template <typename T>
    void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName)
    {
        if (m_SelectionContext.HasComponent<T>())
        {
            if (ImGui::MenuItem(entryName.c_str()))
            {
                m_SelectionContext.AddComponent<T>();
                ImGui::CloseCurrentPopup();
            }
        }
    }
}  // namespace FooGame
