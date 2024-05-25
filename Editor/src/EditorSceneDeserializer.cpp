#include "EditorSceneDeserializer.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <Core.h>
#include <nlohmann/json.hpp>
#include "Log.h"
#include "src/Geometry/AssetLoader.h"
#include "src/Log.h"
#include <tiny_obj_loader.h>
#include <Engine.h>
#include <Log.h>
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
        auto sceneBasePath = canonical(scenePath);
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
#if 0
        for (auto& t : data["textures"])
        {
            std::cout << "Texture " << t << std::endl;
            scene->Textures.emplace_back(AssetLoader::LoadFromFilePtr(t));
        }
        std::cout << "Loaded texture count is  " << scene->Textures.size()
                  << std::endl;
        for (auto& staticMesh : data["staticMeshes"])
        {
            uint32_t id        = staticMesh["id"];
            std::string name   = staticMesh["name"];
            std::string path   = staticMesh["path"];
            std::string format = staticMesh["format"];
            MeshData meshData{};
            FOO_EDITOR_TRACE("Mesh with id: {0}", id);
            FOO_EDITOR_TRACE("\t Name : {0}", name);
            FOO_EDITOR_TRACE("\t Path : {0}", path);
            FOO_EDITOR_TRACE("\t Mesh format : {0}", format);
            if (format == "obj")
            {
                meshData.ModelPtr =
                    AssetLoader::LoadObjModel(staticMesh["path"]);
                ApplyTransformation(meshData, staticMesh);
            }
            else if (format == "gltf" || format == "glb")
            {
                std::string modelFullPath = GetModelFullPath(scenePath, path);
                // meshData.MeshPtr = LoadGLTF(modelFullPath, format == "glb");
                meshData.ModelPtr =
                    AssetLoader::LoadGLTFModel(modelFullPath, true);

                ApplyTransformation(meshData, staticMesh);

                meshData.TextureIndex = staticMesh["textureIndex"];
                // meshData.MeshPtr->SetTexture(0);  // TODO!
                // scene->Meshes.push_back(std::move(meshData));
            }
            else
            {
                FOO_EDITOR_WARN("Format {0} is not supported yet!", format);
                continue;
            }
        }
#endif
        FOO_EDITOR_INFO("Scene data loaded successfully");
        stream.close();
        return std::move(scene);
    }

}  // namespace FooGame
