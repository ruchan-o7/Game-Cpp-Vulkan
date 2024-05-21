#include "EditorScene.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <fstream>
#include <tiny_obj_loader.h>
namespace FooGame
{
    static std::unique_ptr<Mesh> LoadMesh(const String& path)
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
            assert(0 && "Model could not loaded!!!");
        }
        for (const auto& shape : shapes)
        {
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
        return std::make_unique<Mesh>(std::move(vertices), std::move(indices));
    }

    std::unique_ptr<EditorScene> EditorScene::LoadScene(std::ifstream& stream)
    {
        auto scene  = std::make_unique<EditorScene>();
        using json  = nlohmann::json;
        json data   = json::parse(stream, nullptr, false);
        scene->Id   = std::move(data["id"]);
        scene->Name = std::move(data["name"]);
        for (auto& t : data["textures"])
        {
            std::cout << "Texture " << t << std::endl;
            scene->Textures.emplace_back(LoadTexture(t));
        }
        std::cout << "Loaded texture count is  " << scene->Textures.size()
                  << std::endl;
        for (auto& staticMesh : data["staticMeshes"])
        {
            std::cout << "Mesh " << staticMesh["id"] << std::endl;
            std::cout << '\t' << "Name " << staticMesh["name"] << '\n';
            std::cout << '\t' << "Path " << staticMesh["path"] << '\n';
            std::shared_ptr<Mesh> model =
                std::move(LoadMesh(staticMesh["path"]));
            MeshData meshData{};
            meshData.MeshPtr = model;

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

            meshData.TextureIndex = staticMesh["textureIndex"];
            meshData.MeshPtr->SetTexture(
                scene->Textures[meshData.TextureIndex]);
            scene->Meshes.emplace_back(std::move(meshData));
        }
        return scene;
    }
    EditorScene::~EditorScene()
    {
        for (auto& t : Textures)
        {
            DestroyImage(t.get());
            t.reset();
        }
        for (auto& m : Meshes)
        {
            m.MeshPtr.reset();
        }
    }

}  // namespace FooGame
