#include "LoadModel.h"
#include <tiny_obj_loader.h>
#include <memory>
#include "Core/Core/Base.h"
namespace FooGame
{

    std::unique_ptr<Model> LoadModel(const String& path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        String warn, err;
        std::vector<Vertex> vertices;
        std::vector<u32> indices;

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
        std::vector<Mesh> meshes;
        meshes.push_back({std::move(vertices), std::move(indices)});
        return std::make_unique<Model>(std::move(meshes));
    }
}  // namespace FooGame
