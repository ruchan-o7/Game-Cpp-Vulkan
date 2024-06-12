#include "EditorSceneDeserializer.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <Core.h>
#include <nlohmann/json.hpp>
#include "nlohmann/json_fwd.hpp"
#include "src/Geometry/AssetLoader.h"
#include <Log.h>
#include <Engine.h>
#include <tiny_obj_loader.h>
namespace FooGame
{

    using json = nlohmann::json;
    static void ApplyTransformation(MeshData& meshData, json& staticMesh)
    {
        meshData.Transform.Translation.x = staticMesh["transform"]["position"]["x"];
        meshData.Transform.Translation.y = staticMesh["transform"]["position"]["y"];
        meshData.Transform.Translation.z = staticMesh["transform"]["position"]["z"];

        meshData.Transform.Rotation.x = staticMesh["transform"]["rotation"]["x"];
        meshData.Transform.Rotation.y = staticMesh["transform"]["rotation"]["y"];
        meshData.Transform.Rotation.z = staticMesh["transform"]["rotation"]["z"];

        meshData.Transform.Scale.x = staticMesh["transform"]["scale"]["x"];
        meshData.Transform.Scale.y = staticMesh["transform"]["scale"]["y"];
        meshData.Transform.Scale.z = staticMesh["transform"]["scale"]["z"];
    }
    MeshData ProcessSingleAsset(const std::filesystem::path& assetJsonAbsPath, uint32_t assetId)
    {
        MeshData mD{};

        std::ifstream ss(assetJsonAbsPath);
        auto assetJson        = json::parse(ss);
        std::string assetName = assetJson["name"];
        auto modelAbsPath     = assetJsonAbsPath.parent_path() / assetJson["modelPath"];

        FOO_EDITOR_INFO("Asset {0} loading...", assetName);
        if (assetJson["format"] == "obj")
        {
            mD.ModelPtr          = AssetLoader::LoadObjModel(modelAbsPath.string());
            mD.ModelPtr->Name    = assetJson["name"];
            mD.ModelPtr->AssetId = assetId;
            for (int i = 0; i < mD.ModelPtr->m_Meshes.size(); i++)
            {
                auto& mesh             = mD.ModelPtr->m_Meshes[i];
                mesh.materialData.Name = assetJson["meshes"][i]["material"]["name"];
                mesh.materialData.BaseColorTextureIndex =
                    assetJson["meshes"][i]["material"]["baseColorTextureIndex"];
                mesh.materialData.NormalTextureIndex =
                    assetJson["meshes"][i]["material"]["normalColorTextureIndex"];
                mesh.materialData.MetallicRoughnessIndex =
                    assetJson["meshes"][i]["material"]["metallicRoughnessTextureIndex"];
            }
            mD.ModelPtr->Name = assetJson["name"];
            for (const auto& img : assetJson["textures"])
            {
                auto textureAbsPath = assetJsonAbsPath.parent_path() / img["path"];
                auto tex            =std::move( AssetLoader::LoadTexture(textureAbsPath.string()));
                mD.ModelPtr->Textures.push_back(std::move(tex));
                // auto texture = AssetLoader::LoadFromFile(textureAbsPath.string());
                // mD.ModelPtr->images.push_back({texture});
            }
        }
        else if (assetJson["format"] == "glb")
        {
            mD.ModelPtr          = AssetLoader::LoadGLTFModel(modelAbsPath.string(), true);
            mD.ModelPtr->Name    = assetJson["name"];
            mD.ModelPtr->AssetId = assetId;
            for (int i = 0; i < mD.ModelPtr->m_Meshes.size(); i++)
            {
                auto& mesh             = mD.ModelPtr->m_Meshes[i];
                mesh.materialData.Name = assetJson["meshes"][i]["material"]["name"];
                mesh.materialData.BaseColorTextureIndex =
                    assetJson["meshes"][i]["material"]["baseColorTextureIndex"];
                mesh.materialData.NormalTextureIndex =
                    assetJson["meshes"][i]["material"]["normalColorTextureIndex"];
                mesh.materialData.MetallicRoughnessIndex =
                    assetJson["meshes"][i]["material"]["metallicRoughnessTextureIndex"];
            }
        }
        ss.close();
        ApplyTransformation(mD, assetJson);
        return mD;
    }

    std::unique_ptr<EditorScene> EditorSceneDeserializer::DeSerialize(const std::string& scenePath)
    {
        using namespace std::filesystem;
        FOO_EDITOR_INFO("Deserializing scene");
        auto cwd = current_path();
        cwd.append(scenePath);
        auto sceneBasePath = std::filesystem::canonical(cwd);
        std::ifstream stream(sceneBasePath);

        auto scene = std::make_unique<EditorScene>();
        scene->MeshDatas.clear();
        json data   = json::parse(stream, nullptr, false);
        scene->Id   = std::move(data["id"]);
        scene->Name = data["name"];
        for (auto& asset : data["assets"])
        {
            uint32_t assetId = asset["id"];

            auto assetJsonAbsPath = sceneBasePath.parent_path() / path(asset["path"]);

            scene->MeshDatas.push_back(std::move(ProcessSingleAsset(assetJsonAbsPath, assetId)));
        }
        FOO_EDITOR_INFO("Scene data loaded successfully");
        stream.close();
        return scene;
        return std::move(scene);
    }

}  // namespace FooGame
