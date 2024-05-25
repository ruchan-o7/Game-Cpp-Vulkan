#include "LoadModel.h"
// #include <tiny_obj_loader.h>
// #include <memory>
// #include "../Core/Base.h"
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #include <tiny_gltf.h>
namespace FooGame
{
    // TODO Move this to asset loader
    // std::unique_ptr<Model> LoadModel(const String& path)
    // {
    //     tinyobj::attrib_t attrib;
    //     std::vector<tinyobj::shape_t> shapes;
    //     std::vector<tinyobj::material_t> materials;
    //     String warn, err;
    //     std::vector<Vertex> vertices;
    //     std::vector<u32> indices;

    //     if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
    //                           path.c_str()))
    //     {
    //         assert(0 && "Model could not loaded!!!");
    //     }
    //     for (const auto& shape : shapes)
    //     {
    //         for (const auto& index : shape.mesh.indices)
    //         {
    //             Vertex vertex{};
    //             vertex.Position = {
    //                 attrib.vertices[3 * index.vertex_index + 0],
    //                 attrib.vertices[3 * index.vertex_index + 1],
    //                 attrib.vertices[3 * index.vertex_index + 2],
    //             };
    //             vertex.TexCoord = {
    //                 attrib.texcoords[2 * index.texcoord_index + 0],
    //                 1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
    //             };
    //             vertex.Color = {
    //                 attrib.colors[3 * index.vertex_index + 0],
    //                 attrib.colors[3 * index.vertex_index + 1],
    //                 attrib.colors[3 * index.vertex_index + 2],
    //             };  // im not sure

    //             vertices.push_back(vertex);
    //             indices.push_back(indices.size());
    //         }
    //     }
    //     std::vector<Mesh> meshes;
    //     meshes.push_back({std::move(vertices), std::move(indices)});
    //     return std::make_unique<Model>(std::move(meshes));
    // }
    // std::shared_ptr<Mesh> LoadGLTF(const std::string& path, bool isGlb)
    // {
    //     List<Vertex> vertices;
    //     List<uint32_t> indexes;

    //     tinygltf::Model model;
    //     tinygltf::TinyGLTF loader;

    //     std::string err;
    //     std::string warn;
    //     bool ret;
    //     if (isGlb)
    //     {
    //         ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    //     }
    //     else
    //     {
    //         ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    //     }

    //     if (!warn.empty())
    //     {
    //         FOO_CORE_WARN("Warn while loading GLTF model {0}", warn.c_str());
    //     }

    //     if (!err.empty())
    //     {
    //         FOO_CORE_ERROR("Err: {0}", err.c_str());
    //     }

    //     if (!ret)
    //     {
    //         FOO_CORE_ERROR("Failed to parse glTF");
    //     }

    //     for (const auto& mesh : model.meshes)
    //     {
    //         FOO_CORE_TRACE("Parsing Mesh : {0}", mesh.name);
    //         for (const auto& primitive : mesh.primitives)
    //         {
    //             const tinygltf::Accessor& posAccessor =
    //                 model.accessors[primitive.attributes.find("POSITION")
    //                                     ->second];
    //             const tinygltf::BufferView& posBufferView =
    //                 model.bufferViews[posAccessor.bufferView];
    //             const tinygltf::Buffer& posBuffer =
    //                 model.buffers[posBufferView.buffer];
    //             const float* positions = reinterpret_cast<const float*>(
    //                 &posBuffer.data[posBufferView.byteOffset +
    //                                 posAccessor.byteOffset]);

    //             const tinygltf::Accessor* colAccessor     = nullptr;
    //             const tinygltf::BufferView* colBufferView = nullptr;
    //             const tinygltf::Buffer* colBuffer         = nullptr;
    //             const float* colors                       = nullptr;
    //             if (primitive.attributes.find("COLOR_0") !=
    //                 primitive.attributes.end())
    //             {
    //                 colAccessor =
    //                     &model.accessors[primitive.attributes.find("COLOR_0")
    //                                          ->second];
    //                 colBufferView =
    //                 &model.bufferViews[colAccessor->bufferView]; colBuffer =
    //                 &model.buffers[colBufferView->buffer]; colors        =
    //                 reinterpret_cast<const float*>(
    //                     &colBuffer->data[colBufferView->byteOffset +
    //                                      colAccessor->byteOffset]);
    //             }

    //             const tinygltf::Accessor* texAccessor     = nullptr;
    //             const tinygltf::BufferView* texBufferView = nullptr;
    //             const tinygltf::Buffer* texBuffer         = nullptr;
    //             const float* texCoords                    = nullptr;
    //             if (primitive.attributes.find("TEXCOORD_0") !=
    //                 primitive.attributes.end())
    //             {
    //                 texAccessor =
    //                     &model.accessors
    //                          [primitive.attributes.find("TEXCOORD_0")->second];
    //                 texBufferView =
    //                 &model.bufferViews[texAccessor->bufferView]; texBuffer =
    //                 &model.buffers[texBufferView->buffer]; texCoords     =
    //                 reinterpret_cast<const float*>(
    //                     &texBuffer->data[texBufferView->byteOffset +
    //                                      texAccessor->byteOffset]);
    //             }

    //             vertices.reserve(posAccessor.count);
    //             for (size_t i = 0; i < posAccessor.count; ++i)
    //             {
    //                 Vertex vertex;
    //                 vertex.Position =
    //                     glm::vec3(positions[3 * i], positions[3 * i + 1],
    //                               positions[3 * i + 2]);

    //                 if (colors)
    //                 {
    //                     vertex.Color =
    //                         glm::vec3(colors[3 * i], colors[3 * i + 1],
    //                                   colors[3 * i + 2]);
    //                 }
    //                 else
    //                 {
    //                     vertex.Color = glm::vec3(1.0f, 1.0f, 1.0f);
    //                 }

    //                 if (texCoords)
    //                 {
    //                     vertex.TexCoord =
    //                         glm::vec2(texCoords[2 * i], texCoords[2 * i +
    //                         1]);
    //                 }
    //                 else
    //                 {
    //                     vertex.TexCoord = glm::vec2(0.0f, 0.0f);
    //                 }
    //                 vertices.push_back(vertex);
    //             }
    //         }
    //     }
    //     return std::make_shared<Mesh>(std::move(vertices),
    //     std::move(indexes));
    // }
}  // namespace FooGame
