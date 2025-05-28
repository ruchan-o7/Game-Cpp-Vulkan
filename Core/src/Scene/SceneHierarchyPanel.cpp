#include "SceneHierarchyPanel.h"
#include <pch.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <filesystem>
#include "Entity.h"
#include "Scene.h"
#include "Component.h"
#include "../Base.h"
#include "../Core/AssetManager.h"
#include "../Scripts/CameraController.h"
#include "../Scripts/Rotate.h"
#include "../Scripts/ScaleYoink.h"
#include "../Base.h"
#include "../Core/File.h"
#include "../Core/ObjLoader.h"
#include "../Base.h"
#include "../Scene/Asset.h"
#include "../Core/GltfLoader.h"
#include "../Config.h"
#include "../Engine/Engine/Renderer3D.h"
#include "../Scene/AssetSerializer.h"
#include "backends/imgui_impl_vulkan.h"
#include "../Engine/Core/VulkanTexture.h"
#include "src/Core/Assert.h"
#include "vulkan/vulkan_core.h"
#include <nlohmann/json.hpp>
namespace FooGame
{
    enum class AssetType
    {
        Material,
        Image,
        Model
    };
    template <class Type>
    struct AssetFile
    {
            std::filesystem::path Asset;
            std::filesystem::path Extra;

            String AssetStr;
            String ExtraStr;
            Type AssetFile;
            bool IsLoaded           = false;
            UUID Id                 = 0;
            VkDescriptorSet Preview = VK_NULL_HANDLE;
    };
    static std::filesystem::path materialsDir;
    static std::filesystem::path imagesDir;
    static std::filesystem::path modelsDir;
    static const std::string scriptNames[] = {"RotateScript", "ScaleYoink", "CameraController"};
    List<AssetFile<Asset::FImage>> imageFiles;
    List<AssetFile<Asset::FModel>> modelFiles;
    List<AssetFile<Asset::FMaterial>> materialFiles;

    AssetType m_SelectedAssetType;
    i32 m_SelectedAssetFileIndex = -1;

    SceneHierarchyPanel::SceneHierarchyPanel(Scene* context)
    {
        SetContext(context);
        materialsDir = File::GetMaterialsPath();
        imagesDir    = File::GetImagesPath();
        modelsDir    = File::GetModelsPath();
        RefreshAssetFiles();
    }

    void SceneHierarchyPanel::SetContext(Scene* context)
    {
        m_pScene = context;
    }

    void SceneHierarchyPanel::RefreshAssetFiles()
    {
        // Images

        for (auto& f : std::filesystem::directory_iterator(imagesDir))
        {
            AssetFile<Asset::FImage> af;
            if (f.path().extension() == FIMAGE_ASSET_EXTENSION)
            {
                af.Asset    = f.path();
                af.AssetStr = af.Asset.filename().replace_extension().string();
                imageFiles.emplace_back(std::move(af));
                continue;
            }
            if (f.path().extension() == FIMAGE_BUFFER_EXTENSION)
            {
                imageFiles[imageFiles.size() - 1].Extra = f.path();
                imageFiles[imageFiles.size() - 1].ExtraStr =
                    f.path().filename().replace_extension().string();
            }
        }
        // Materials
        for (auto& f : std::filesystem::directory_iterator(materialsDir))
        {
            AssetFile<Asset::FMaterial> af;
            DEFER(materialFiles.emplace_back(std::move(af)));
            if (f.path().extension() == FMATERIAL_ASSET_EXTENSION)
            {
                af.Asset    = f.path();
                af.AssetStr = af.Asset.filename().replace_extension().string();
            }
        }

        // Models
        for (auto& f : std::filesystem::directory_iterator(modelsDir))
        {
            AssetFile<Asset::FModel> af;
            DEFER(modelFiles.emplace_back(std::move(af)));
            if (f.path().extension() == FMODEL_ASSET_EXTENSION)
            {
                af.Asset    = f.path();
                af.AssetStr = af.Asset.filename().replace_extension().string();
            }
        }
    }
    void SceneHierarchyPanel::DrawAssets()
    {
        ImGui::Begin("Assets");
        DEFER(ImGui::End());
        if (ImGui::BeginTabBar("_assets_panel_"))
        {
            DEFER(ImGui::EndTabBar());

            if (ImGui::BeginPopupContextItem("_assets_pop_context_"))
            {
                DEFER(ImGui::EndPopup());
                if (ImGui::Selectable("Refresh"))
                {
                    RefreshAssetFiles();
                }

                if (ImGui::Selectable("Add Material"))
                {
                }
                if (ImGui::Selectable("Add Image/Texture"))
                {
                    auto image = File::OpenFileDialog(
                        "Image file (*.png;*.jpg;*.jpeg)\0*.png\0*.jpg\0*.jpeg");
                    AssetManager::LoadExternalImage(image);
                    RefreshAssetFiles();
                }
                if (ImGui::Selectable("Add Model"))
                {
                }
            }

            if (ImGui::BeginTabItem("Materials"))
            {
                DEFER(ImGui::EndTabItem());
                for (size_t i = 0; i < materialFiles.size(); i++)
                {
                    if (ImGui::Selectable(materialFiles[i].AssetStr.c_str()))
                    {
                        m_SelectedAssetFileIndex = i;
                        m_SelectedAssetType      = AssetType::Material;
                        if (!materialFiles[i].IsLoaded)
                        {
                            std::ifstream is{materialFiles[i].Asset};
                            DEFER(is.close());
                            nlohmann::json js = nlohmann::json::parse(is);
                            MaterialSerializer ms;
                            materialFiles[i].AssetFile = std::move(ms.DeSerialize(js));
                            materialFiles[i].IsLoaded  = true;
                        }
                    }
                }
            }
            if (ImGui::BeginTabItem("Images"))
            {
                DEFER(ImGui::EndTabItem());

                for (size_t i = 0; i < imageFiles.size(); i++)
                {
                    auto& imFile = imageFiles[i];

                    if (ImGui::Selectable(imFile.AssetStr.c_str()))
                    {
                        m_SelectedAssetFileIndex = i;
                        m_SelectedAssetType      = AssetType::Image;
                        if (!imFile.IsLoaded)
                        {
                            std::ifstream is{imFile.Asset};
                            DEFER(is.close());
                            nlohmann::json js = nlohmann::json::parse(is);
                            ImageSerializer ms;
                            imFile.AssetFile = std::move(ms.DeSerialize(js));
                            imFile.IsLoaded  = true;
                        }
                    }
                }
            }
            if (ImGui::BeginTabItem("Models"))
            {
                DEFER(ImGui::EndTabItem());
                for (size_t i = 0; i < modelFiles.size(); i++)
                {
                    if (ImGui::Selectable(modelFiles[i].AssetStr.c_str()))
                    {
                        m_SelectedAssetFileIndex = i;
                        m_SelectedAssetType      = AssetType::Model;
                        if (!modelFiles[i].IsLoaded)
                        {
                            std::ifstream is{modelFiles[i].Asset};
                            DEFER(is.close());
                            nlohmann::json js = nlohmann::json::parse(is);
                            ModelSerializer ms;
                            modelFiles[i].AssetFile = std::move(ms.DeSerialize(js));
                            modelFiles[i].IsLoaded  = true;
                        }
                    }
                }
            }
        }
    }

    void SceneHierarchyPanel::OnImgui()
    {
        ImGui::Begin("Scene hierarchy");
        DEFER(ImGui::End());
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
                m_SelectedMaterial = 0;
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
        DEFER(ImGui::End());
        if (m_SelectionContext)
        {
            DrawComponents(m_SelectionContext);
        }

        // DrawMaterialSection();
        ImGui::Begin("Asset properties");
        DEFER(ImGui::End());

        DrawAssetProperty();
        DrawAssets();
    }
    void SceneHierarchyPanel::DrawAssetProperty()
    {
        if (m_SelectedAssetFileIndex == -1)
        {
            return;
        }
        switch (m_SelectedAssetType)
        {
            case AssetType::Material:
            {
                DrawMaterial();
                break;
            }
            case AssetType::Image:
            {
                DrawImage();
                break;
            }
            case AssetType::Model:
            {
                DrawModel();
                break;
            }
        }
    }
    void SceneHierarchyPanel::DrawMaterial()
    {
        auto avail = ImGui::GetContentRegionAvail();

        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        ImGui::SeparatorText("Material Properties");
        // auto* materialAsset = AssetManager::GetMaterialAsset(m_SelectedMaterial);
        // auto material       = materialAsset->Asset;
        auto& materialFile = materialFiles[m_SelectedAssetFileIndex];
        auto& material     = materialFile.AssetFile;
        strncpy_s(buffer, sizeof(buffer), material.Name.c_str(), sizeof(buffer));
        if (materialFile.Id != DEFAULT_MATERIAL_ID)
        {
            if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
            {
                material.Name = std::string(buffer);
                // materialAsset->Name = std::string(buffer);
            }
        }
        else
        {
            ImGui::Text("%s", material.Name.c_str());
        }
        ImGui::Text("Base color: %s", material.BaseColorTexture.Name.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Change"))
        {
            ImGui::OpenPopup("texture_popup_base");
        }
        if (ImGui::BeginPopup("texture_popup_base"))
        {
            ImGui::SeparatorText("All Textures");
            DEFER(ImGui::EndPopup());
            auto textures = AssetManager::GetAllImages();
            for (auto& [id, tex] : textures)
            {
                if (ImGui::Selectable(tex.Name.c_str()))
                {
                    material.BaseColorTexture.id   = id;
                    material.BaseColorTexture.Name = String(tex.Name);
                }
            }
        }
    }
    void SceneHierarchyPanel::DrawImage()
    {
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        ImGui::SeparatorText("Image Properties");
        auto& imageFile = imageFiles[m_SelectedAssetFileIndex];
        auto& image     = imageFile.AssetFile;
        strncpy_s(buffer, sizeof(buffer), image.Name.c_str(), sizeof(buffer));
        if (imageFile.Id != DEFAULT_MATERIAL_ID || imageFile.Id != 0)
        {
            if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
            {
                image.Name = std::string(buffer);
                // materialAsset->Name = std::string(buffer);
            }
            if (imageFile.Preview == VK_NULL_HANDLE)
            {
                FOO_CORE_INFO("Image asset preview is null so uploading");

                AssetManager::LoadFIMG(imageFile.AssetFile, imageFile.Id);

                auto& loadedImage = AssetManager::GetTextureAsset(imageFile.Id)->Asset;

                imageFile.Preview = ImGui_ImplVulkan_AddTexture(
                    loadedImage->DescriptorInfo.sampler, loadedImage->DescriptorInfo.imageView,
                    loadedImage->DescriptorInfo.imageLayout);
                FOO_ASSERT(imageFile.Preview != VK_NULL_HANDLE);
            }
            ImGui::Image(imageFile.Preview, ImVec2(100, 100));
            ImGui::SameLine();
            ImGui::Text("Width: %d", imageFile.AssetFile.Width);
            ImGui::Text("Height: %d", imageFile.AssetFile.Height);
        }
        else
        {
            ImGui::Text("%s", image.Name.c_str());
        }
    }
    void SceneHierarchyPanel::DrawModel()
    {
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        ImGui::SeparatorText("Model Properties");
        auto& modelFile = modelFiles[m_SelectedAssetFileIndex];
        auto& model     = modelFile.AssetFile;
        strncpy_s(buffer, sizeof(buffer), model.Name.c_str(), sizeof(buffer));
        if (modelFile.Id != DEFAULT_MATERIAL_ID)
        {
            if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
            {
                model.Name = std::string(buffer);
            }
        }
        else
        {
            ImGui::Text("%s", model.Name.c_str());
        }
    }
    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags =
            ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
            ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        bool opened =
            ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());
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
            DisplayAddComponentEntry<ModelRendererComponent>("Mesh Renderer");

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
                        component.Bind(scriptNames[0]);
                        ImGui::CloseCurrentPopup();
                    }
                    else if (ImGui::MenuItem(scriptNames[1].c_str()))
                    {
                        component.Bind(scriptNames[1]);
                        ImGui::CloseCurrentPopup();
                    }
                    else if (ImGui::MenuItem(scriptNames[2].c_str()))
                    {
                        component.Bind(scriptNames[2]);

                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            });
        auto& sceneMaterials = AssetManager::GetAllMaterials();
        DrawComponent<ModelRendererComponent>(
            "Mesh Renderer", entity,
            [&](ModelRendererComponent& component)
            {
                auto assetModel = AssetManager::GetModelAsset(component.AssetModelId);

                ImGui::Text("Model: %s",
                            assetModel == nullptr ? "No model" : assetModel->Name.c_str());
                if (!assetModel)
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
                                auto fileNameStr = o.filename().string();
                                ObjLoader loader{o};
                                auto objModel = loader.LoadModel();
                                if (objModel)
                                {
                                    UUID id;
                                    AssetManager::LoadObjModel(std::move(objModel), id);
                                    component.AssetModelId = id;
                                }
                            }
                        }
                        if (ImGui::MenuItem("Load Gltf"))
                        {
                            List<std::filesystem::path> gltfs;
                            File::OpenFileDialog(gltfs);
                            bool isOk = false;
                            for (auto& g : gltfs)
                            {
                                if (g.extension() == ".glb" || g.extension() == ".gltf")
                                {
                                    isOk = true;
                                    break;
                                }
                            }
                            if (!isOk)
                            {
                                return;
                            }
                            for (auto& g : gltfs)
                            {
                                GltfLoader loader{g.string(), g.extension() == ".glb"};
                                UUID id;
                                AssetManager::LoadGLTFModelAsync(loader, id);
                                component.AssetModelId = id;
                            }
                        }
                        ImGui::EndPopup();
                    }
                    return;
                }
                else
                {
                    auto model = assetModel->Asset;

                    ImGui::Text("Total vertex count: %zu", model->Vertices.size());
                    ImGui::Text("Total index count: %zu", model->Indices.size());
                    ImGui::Text("Total Mesh count: %zu", model->Meshes.size());
                    i32 primitiveCount = 0;
                    for (const auto& m : model->Meshes)
                    {
                        for (const auto& p : m.Primitives)
                        {
                            primitiveCount++;
                        }
                    }
                    ImGui::Text("Total primitive count: %i", primitiveCount);
                    if (ImGui::TreeNode("Meshes"))
                    {
                        i32 meshCount = model->Meshes.size();
                        DEFER(ImGui::TreePop());
                        auto size = ImGui::GetContentRegionAvail();
                        for (size_t meshIndex = 0; meshIndex < meshCount; meshIndex++)
                        {
                            auto& mesh = model->Meshes[meshIndex];

                            if (ImGui::TreeNode(mesh.Name.c_str()))
                            {
                                size_t primitiveCount = mesh.Primitives.size();
                                DEFER(ImGui::TreePop());
                                for (size_t primitiveIndex = 0; primitiveIndex < primitiveCount;
                                     primitiveIndex++)
                                {
                                    auto& primitive =
                                        model->Meshes[meshIndex].Primitives[primitiveIndex];
                                    auto materialId = primitive.MaterialId;
                                    auto* material  = AssetManager::GetMaterialAsset(materialId);
                                    if (ImGui::TreeNode("", "Primitive Index: %zu", primitiveIndex))
                                    {
                                        DEFER(ImGui::TreePop());
                                        ImGui::Text("Material: %s", material->Name.c_str());
                                        ImGui::SameLine();
                                        if (ImGui::Button("Change"))
                                        {
                                            ImGui::OpenPopup("change_material_popup");
                                        }
                                        if (ImGui::BeginPopup("change_material_popup"))
                                        {
                                            DEFER(ImGui::EndPopup());
                                            ImGui::SeparatorText("Materials");
                                            for (const auto& [id, asset] : sceneMaterials)
                                            {
                                                if (ImGui::Selectable(asset.Name.c_str()))
                                                {
                                                    primitive.MaterialId = id;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
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
