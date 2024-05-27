#include "EditorSceneDeserializer.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <Core.h>
#include <nlohmann/json.hpp>
#include <Log.h>
#include <Engine.h>
#include <tiny_obj_loader.h>
namespace FooGame
{

    static void ApplyTransformation(MeshData& meshData,
                                    nlohmann::basic_json<>& staticMesh)
    {
        meshData.Transform.Translation.x =
            staticMesh["transform"]["position"]["x"];
        meshData.Transform.Translation.y =
            staticMesh["transform"]["position"]["y"];
        meshData.Transform.Translation.z =
            staticMesh["transform"]["position"]["z"];

        meshData.Transform.Rotation.x =
            staticMesh["transform"]["rotation"]["x"];
        meshData.Transform.Rotation.y =
            staticMesh["transform"]["rotation"]["y"];
        meshData.Transform.Rotation.z =
            staticMesh["transform"]["rotation"]["z"];

        meshData.Transform.Scale.x = staticMesh["transform"]["scale"]["x"];
        meshData.Transform.Scale.y = staticMesh["transform"]["scale"]["y"];
        meshData.Transform.Scale.z = staticMesh["transform"]["scale"]["z"];
    }

    std::unique_ptr<EditorScene> EditorSceneDeserializer::DeSerialize(
        const std::string& scenePath)
    {
        using namespace std::filesystem;
        using json = nlohmann::json;
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
            MeshData meshData{};
            uint32_t assetId = asset["id"];

            auto assetRelativePath =
                sceneBasePath.parent_path() / path(asset["path"]);

            std::ifstream ss(assetRelativePath);

            auto assetJson        = json::parse(ss);
            std::string assetName = assetJson["name"];
            FOO_EDITOR_INFO("Asset {0} loading...", assetName);
            auto assetAbsPath =
                assetRelativePath.parent_path() / assetJson["modelPath"];
            if (assetJson["format"] == "obj")
            {
                meshData.ModelPtr =
                    AssetLoader::LoadObjModel(assetAbsPath.string());
                meshData.ModelPtr->AssetId = assetId;
                for (int i = 0; i < meshData.ModelPtr->m_Meshes.size(); i++)
                {
                    auto& mesh = meshData.ModelPtr->m_Meshes[i];
                    mesh.materialData.Name =
                        assetJson["meshes"][i]["material"]["name"];
                    mesh.materialData.BaseColorTextureIndex =
                        assetJson["meshes"][i]["material"]
                                 ["baseColorTextureIndex"];
                    mesh.materialData.NormalTextureIndex =
                        assetJson["meshes"][i]["material"]
                                 ["normalColorTextureIndex"];
                    mesh.materialData.MetallicRoughnessIndex =
                        assetJson["meshes"][i]["material"]
                                 ["metallicRoughnessTextureIndex"];
                }
                for (const auto& img : assetJson["textures"])
                {
                    auto textureAbsPath =
                        assetRelativePath.parent_path() / img["path"];
                    auto texture =
                        AssetLoader::LoadFromFile(textureAbsPath.string());
                    meshData.ModelPtr->images.push_back({texture});
                }
            }
            else if (assetJson["format"] == "glb")
            {
                meshData.ModelPtr =
                    AssetLoader::LoadGLTFModel(assetAbsPath.string(), true);
                for (int i = 0; i < meshData.ModelPtr->m_Meshes.size(); i++)
                {
                    auto& mesh = meshData.ModelPtr->m_Meshes[i];
                    mesh.materialData.Name =
                        assetJson["meshes"][i]["material"]["name"];
                    mesh.materialData.BaseColorTextureIndex =
                        assetJson["meshes"][i]["material"]
                                 ["baseColorTextureIndex"];
                    mesh.materialData.NormalTextureIndex =
                        assetJson["meshes"][i]["material"]
                                 ["normalColorTextureIndex"];
                    mesh.materialData.MetallicRoughnessIndex =
                        assetJson["meshes"][i]["material"]
                                 ["metallicRoughnessTextureIndex"];
                }
            }
            scene->MeshDatas.push_back(meshData);
            ss.close();
        }
        FOO_EDITOR_INFO("Scene data loaded successfully");
        stream.close();
        return std::move(scene);
    }

}  // namespace FooGame
