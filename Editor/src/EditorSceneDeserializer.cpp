#include "EditorSceneDeserializer.h"
#include <vector>
#include <filesystem>
#include <fstream>
#include <memory>
#include <Core.h>
#include <nlohmann/json.hpp>
#include <tiny_obj_loader.h>
#include <Engine.h>
#include <Log.h>
namespace FooGame
{

    static std::shared_ptr<Mesh> LoadObjMesh(const String& path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        std::vector<Vertex> vertices;

        std::vector<uint32_t> indices;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              path.c_str()))
        {
            FOO_EDITOR_CRITICAL("Model could not loaded!!!");
            std::cin.get();
        }
        for (const auto& shape : shapes)
        {
            vertices.reserve(shape.mesh.indices.size());
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};
                vertex.Position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };
                vertex.TexCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                };
                vertex.Color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };  // im not sure
                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
        }
        return std::make_shared<Mesh>(std::move(vertices), std::move(indices));
    }
    std::string GetModelFullPath(const std::string& sceneJsonPath,
                                 const std::string& modelRelativePath)
    {
        std::filesystem::path scenePath(sceneJsonPath);

        std::filesystem::path modelPath =
            scenePath.parent_path() / modelRelativePath;

        return std::move(modelPath.string());
    }
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
        using json = nlohmann::json;
        FOO_EDITOR_INFO("Deserializing scene");
        std::ifstream stream(scenePath);

        auto scene = std::make_unique<EditorScene>();
        scene->Meshes.clear();
        json data   = json::parse(stream, nullptr, false);
        scene->Id   = std::move(data["id"]);
        scene->Name = data["name"];
        for (auto& t : data["textures"])
        {
            std::cout << "Texture " << t << std::endl;
            scene->Textures.emplace_back(LoadTexture(t));
        }
        std::cout << "Loaded texture count is  " << scene->Textures.size()
                  << std::endl;
        scene->Meshes.reserve(data["staticMeshes"].size());
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
                meshData.MeshPtr = LoadObjMesh(staticMesh["path"]);
                ApplyTransformation(meshData, staticMesh);
                meshData.TextureIndex = staticMesh["textureIndex"];
                meshData.MeshPtr->SetTexture(
                    scene->Textures[meshData.TextureIndex]);
                scene->Meshes.push_back(std::move(meshData));
            }
            else if (format == "gltf" || format == "glb")
            {
                std::string modelFullPath = GetModelFullPath(scenePath, path);
                // meshData.MeshPtr = LoadGLTF(modelFullPath, format == "glb");
                meshData.MeshPtr =
                    AssetLoader::LoadGLTFMesh(modelFullPath, true);
                ApplyTransformation(meshData, staticMesh);
                // TODO FOR NOW
                meshData.TextureIndex = staticMesh["textureIndex"];
                meshData.MeshPtr->SetTexture(0);  // TODO!
                scene->Meshes.push_back(std::move(meshData));
            }
            else
            {
                FOO_EDITOR_WARN("Format {0} is not supported yet!", format);
                continue;
            }
        }
        FOO_EDITOR_INFO("Scene data loaded successfully");
        stream.close();
        size_t vertexSize = 0;
        size_t indexSize  = 0;
        for (auto& mesh : scene->Meshes)
        {
            vertexSize += mesh.MeshPtr->m_Vertices.size() * sizeof(Vertex);
            indexSize  += mesh.MeshPtr->m_Indices.size() * sizeof(uint32_t);
        }
        FOO_EDITOR_INFO("Will allocate {0} of bytes for vertices", vertexSize);
        FOO_EDITOR_INFO("Will allocate {0} of bytes for indices", indexSize);
        return std::move(scene);
    }

}  // namespace FooGame
