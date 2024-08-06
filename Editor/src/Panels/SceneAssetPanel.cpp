#include "SceneAssetPanel.h"
#include <cstdint>
#include <filesystem>
#include "imgui.h"
#include "json.hpp"
#include "src/Base.h"
#include "src/Core/AssetManager.h"
#include "src/Scene/Asset.h"
#include "src/Scene/AssetSerializer.h"
namespace FooGame
{

    using Path = std::filesystem::path;
    static void DrawMaterial();
    static void DrawImage();
    static void DrawModel();
    enum class AssetType : std::uint8_t
    {
        None,
        Material,
        Image,
        Model
    };

    template <class T>
    struct Item
    {
            T* Ptr        = nullptr;
            bool IsLoaded = false;
            String Name;
            Path FilePath;
            std::filesystem::file_time_type LastWrite;
    };
    struct ProjectContext
    {
            Path WorkingDirectory;
            Path AssetDirectory;

            i32 ImageAssetCount    = 0;
            i32 MaterialAssetCount = 0;
            i32 ModelAssetCount    = 0;
            List<Item<AssetTextureC>> Images;
            List<Item<AssetMaterialC>> Materials;
            List<Item<AssetModelC>> Models;
            i32 SelectedItemPos    = -1;
            UUID SelectedAssetId   = 0;
            AssetType SelectedType = AssetType::None;
    };
    static ProjectContext pContext;
    void SetSelectedItem(i32 pos, AssetType type)
    {
        switch (type)
        {
            case AssetType::Material:
            {
                auto& materialFile = pContext.Materials[pos];
                if (!materialFile.IsLoaded)
                {
                    nlohmann::json json;
                    File::ReadJsonData(materialFile.FilePath, json);
                    MaterialSerializer matSer;
                    auto fmat = matSer.DeSerialize(json);
                    AssetManager::AddMaterial(std::move(fmat));
                }
                break;
            }
            case AssetType::Image:
            {
                auto& imageFile = pContext.Images[pos];
                if (!imageFile.IsLoaded)
                {
                    nlohmann::json json;
                    File::ReadJsonData(imageFile.FilePath, json);
                    ImageSerializer imSer;
                    auto fimg = imSer.DeSerialize(json);
                    AssetManager::LoadFIMG(fimg, fimg.Id);
                }
                break;
            }
            case AssetType::Model:
            {
                break;  // TODO: Remove this when feed data
                auto& modelFile = pContext.Models[pos];
                if (!modelFile.IsLoaded)
                {
                    nlohmann::json json;
                    File::ReadJsonData(modelFile.FilePath, json);
                    ModelSerializer modelSer;
                    auto fmodel = modelSer.DeSerialize(json);
                    List<Vertex> vertices;  // TODO: Feed from original data file
                    List<u32> indices;
                    AssetManager::LoadModel(fmodel, fmodel.Id, std::move(vertices),
                                            std::move(indices));
                }
                break;
            }
            case AssetType::None:
                break;
        }
    }

    AssetPanel::AssetPanel(const Path& workingDirectory)
    {
        pContext.WorkingDirectory = workingDirectory;
#ifdef FOO_DEBUG
        pContext.AssetDirectory = workingDirectory / "Assets/Scenes/Prototype3/Assets";
#else
        pContext.AssetDirectory = workingDirectory / "Assets";
#endif
        Refresh();
    }
    static void ResetMaterials()
    {
        pContext.MaterialAssetCount = 0;
        pContext.Materials.clear();
    }
    static void ResetModels()
    {
        pContext.ModelAssetCount = 0;
        pContext.Models.clear();
    }
    static void ResetImages()
    {
        pContext.ImageAssetCount = 0;
        pContext.Images.clear();
    }

    static void ResetRegistiries()
    {
        ResetMaterials();
        ResetModels();
        ResetImages();
    }

    static AssetTextureC* FindImage(const String& name)
    {
        for (const auto& [id, asset] : AssetManager::GetAllImages())
        {
            if (asset.Asset->GetName() == name)
            {
                return AssetManager::GetTextureAsset(id);
            }
        }
        return nullptr;
    }
    static AssetMaterialC* FindMaterial(const String& name)
    {
        for (const auto& [id, asset] : AssetManager::GetAllMaterials())
        {
            if (asset.Asset.Name == name)
            {
                return AssetManager::GetMaterialAsset(id);
            }
        }
        return nullptr;
    }
    static AssetModelC* FindModel(const String& name)
    {
        for (const auto& [id, asset] : AssetManager::GetAllModels())
        {
            if (asset.Asset.Name == name)
            {
                return AssetManager::GetModelAsset(id);
            }
        }
        return nullptr;
    }

    void AssetPanel::Refresh()
    {
        auto imageDir    = pContext.AssetDirectory / "Images";
        auto materialDir = pContext.AssetDirectory / "Materials";
        auto modelDir    = pContext.AssetDirectory / "Models";
        ResetRegistiries();

        for (const auto& dir : std::filesystem::directory_iterator(imageDir))
        {
            if (dir.path().extension() != FIMAGE_ASSET_EXTENSION)
            {
                continue;
            }

            pContext.ImageAssetCount++;
            Item<AssetTextureC> item{};
            item.Ptr       = FindImage(dir.path().filename().string());
            item.IsLoaded  = item.Ptr != nullptr;
            item.Name      = dir.path().filename().string();
            item.FilePath  = dir.path();
            item.LastWrite = dir.last_write_time();
            pContext.Images.emplace_back(std::move(item));
        }
        for (const auto& dir : std::filesystem::directory_iterator(modelDir))
        {
            if (dir.path().extension() != FMODEL_ASSET_EXTENSION)
            {
                continue;
            }
            pContext.ModelAssetCount++;
            Item<AssetModelC> item{};
            item.Ptr      = FindModel(dir.path().filename().string());
            item.IsLoaded = item.Ptr != nullptr;
            item.Name     = dir.path().filename().string();
            item.FilePath = dir.path();
            pContext.Models.emplace_back(std::move(item));
        }
        for (const auto& dir : std::filesystem::directory_iterator(materialDir))
        {
            if (dir.path().extension() != FMATERIAL_ASSET_EXTENSION)
            {
                continue;
            }
            pContext.MaterialAssetCount++;
            Item<AssetMaterialC> item{};
            item.Ptr      = FindMaterial(dir.path().filename().string());
            item.IsLoaded = item.Ptr != nullptr;
            item.Name     = dir.path().filename().string();
            item.FilePath = dir.path();
            pContext.Materials.emplace_back(std::move(item));
        }
    }

    void AssetPanel::OnImGui()
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
                    Refresh();
                }

                if (ImGui::Selectable("Add Material"))
                {
                    auto fileName = pContext.AssetDirectory / "Materials" / "New material.fmat";
                    Asset::FMaterial mat;
                    mat.Name = "New Material";
                    mat.Id   = UUID();
                    MaterialSerializer mats;
                    nlohmann::json json = mats.Serialize(mat);
                    File::WriteJsonData(fileName, FMATERIAL_ASSET_EXTENSION, json);
                    Refresh();
                }
                if (ImGui::Selectable("Add Image/Texture"))
                {
                    auto image = File::OpenFileDialog(
                        "Image file (*.png;*.jpg;*.jpeg)\0*.png\0*.jpg\0*.jpeg");
                    AssetManager::LoadExternalImage(image);
                }
                if (ImGui::Selectable("Add Model"))
                {
                }
            }
#define NO_FILE() ImGui::Text("%s", "Empty.")
            if (ImGui::BeginTabItem("Materials"))
            {
                i32 count = 0;
                DEFER(ImGui::EndTabItem());
                if (pContext.MaterialAssetCount == 0)
                {
                    NO_FILE();
                }
                else
                {
                    for (const auto& m : pContext.Materials)
                    {
                        count++;
                        if (ImGui::Selectable(m.Name.c_str()))
                        {
                            pContext.SelectedItemPos = count;
                        }
                    }
                }
            }
            if (ImGui::BeginTabItem("Images"))
            {
                DEFER(ImGui::EndTabItem());
                if (pContext.ImageAssetCount == 0)
                {
                    NO_FILE();
                }
                else
                {
                    for (const auto& m : pContext.Images)
                    {
                        if (ImGui::Selectable(m.Name.c_str()))
                        {
                        }
                    }
                }
            }
            if (ImGui::BeginTabItem("Models"))
            {
                DEFER(ImGui::EndTabItem());
                if (pContext.ModelAssetCount == 0)
                {
                    NO_FILE();
                }
                else
                {
                    for (const auto& m : pContext.Models)
                    {
                        if (ImGui::Selectable(m.Name.c_str()))
                        {
                        }
                    }
                }
            }
        }
        if (pContext.SelectedAssetId != 0)
        {
            return;
        }

        switch (pContext.SelectedType)
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
            default:
                break;
        }
    }

    static void DrawMaterial()
    {
        auto avail = ImGui::GetContentRegionAvail();

        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        ImGui::SeparatorText("Material Properties");

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
            }
        }
    }
    void DrawImage()
    {
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        ImGui::SeparatorText("Image Properties");
    }
    void DrawModel()
    {
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        ImGui::SeparatorText("Model Properties");
    }
}  // namespace FooGame