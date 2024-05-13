#include "Model.h"
#include <cassert>
#include "../Backend/Vertex.h"
#include "Core/Core/Base.h"
#include "tiny_obj_loader.h"
namespace FooGame
{
    Shared<Model> Model::LoadModel(const String& path)
    {
        tinyobj::attrib_t attrib;
        List<tinyobj::shape_t> shapes;
        List<tinyobj::material_t> materials;
        String warn, err;
        List<Vertex> vertices;
        List<u32> indices;

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
        List<Mesh> meshes;
        meshes.push_back({std::move(vertices), std::move(indices)});
        return CreateShared<Model>(std::move(meshes));
    }
    Mesh::Mesh(List<Vertex>&& vertices, List<u32>&& indices)
        : m_Vertices(std::move(vertices)), m_Indices(std::move(indices))
    {
    }
    Model::Model(List<Mesh>&& meshes) : m_Meshes(std::move(meshes))
    {
    }
}  // namespace FooGame
