#include "AssetLoader.h"
#include <Log.h>
#include <glm/gtc/type_ptr.hpp>
namespace FooGame
{

    Unique<Mesh> AssetLoader::LoadGLTFMesh(const String& path, bool isGlb)
    {
        tinygltf::Model gltfInput;
        tinygltf::TinyGLTF gltfContext;
        String error, warning;
        bool ret;
        if (isGlb)
        {
            ret = gltfContext.LoadBinaryFromFile(&gltfInput, &error, &warning,
                                                 path);
        }
        else
        {
            ret = gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning,
                                                path);
        }

        if (!warning.empty())
        {
            FOO_CORE_WARN("Warn while loading GLTF model {0}", warning.c_str());
        }

        if (!error.empty())
        {
            FOO_CORE_ERROR("Err: {0}", error.c_str());
        }

        if (!ret)
        {
            FOO_CORE_ERROR("Failed to parse glTF");
        }
        List<u32> indices;
        List<GLTF::Vertex> vertices;
        GLTF::GLTFModel gltfModel;
        const tinygltf::Scene& scene = gltfInput.scenes[0];
        for (size_t i = 0; i < scene.nodes.size(); i++)
        {
            const auto node = gltfInput.nodes[scene.nodes[i]];

            gltfModel.LoadNode(node, gltfInput, nullptr, indices, vertices);
        }
        List<Vertex> vertices_;
        vertices_.reserve(vertices.size());
        for (size_t i = 0; i < vertices.size(); i++)
        {
            Vertex v{};
            v.Position = vertices[i].Position;
            v.Color    = vertices[i].Color;
            v.TexCoord = vertices[i].Uv;
            vertices_.push_back(v);
        }

        return CreateUnique<Mesh>(std::move(vertices_), std::move(indices));
    }
    namespace GLTF
    {

        void GLTFModel::LoadNode(const tinygltf::Node& inputNode,
                                 const tinygltf::Model& input, Node* parent,
                                 List<u32>& indices,
                                 List<GLTF::Vertex>& vertices)
        {
            GLTF::Node* node = new GLTF::Node{};
            node->Matrix     = glm::mat4(1.0f);
            node->Parent     = parent;
            if (inputNode.translation.size() == 3)
            {
                node->Matrix = glm::translate(
                    node->Matrix,
                    glm::vec3(glm::make_vec3(inputNode.translation.data())));
            }
            if (inputNode.rotation.size() == 4)
            {
                glm::quat q   = glm::make_quat(inputNode.rotation.data());
                node->Matrix *= glm::mat4(q);
            }
            if (inputNode.scale.size() == 3)
            {
                node->Matrix = glm::scale(
                    node->Matrix,
                    glm::vec3(glm::make_vec3(inputNode.scale.data())));
            }
            if (inputNode.matrix.size() == 16)
            {
                node->Matrix = glm::make_mat4x4(inputNode.matrix.data());
            }
            if (inputNode.children.size() > 0)
            {
                for (size_t i = 0; i < inputNode.children.size(); i++)
                {
                    LoadNode(input.nodes[inputNode.children[i]], input, node,
                             indices, vertices);
                }
            }
            if (inputNode.mesh > -1)
            {
                const auto mesh = input.meshes[inputNode.mesh];
                for (size_t i = 0; i < mesh.primitives.size(); i++)
                {
                    const auto& gltfPrimitive = mesh.primitives[i];
                    u32 firstIndex  = static_cast<u32>(indices.size());
                    u32 vertexStart = static_cast<u32>(vertices.size());
                    u32 indexCount  = 0;

                    {
                        const float* positionBuffer  = nullptr;
                        const float* normalsBuffer   = nullptr;
                        const float* texCoordsBuffer = nullptr;
                        size_t vertexCount           = 0;

                        if (gltfPrimitive.attributes.find("POSITION") !=
                            gltfPrimitive.attributes.end())
                        {
                            const auto& accessor =
                                input.accessors[gltfPrimitive.attributes
                                                    .find("POSITION")
                                                    ->second];
                            const auto& view =
                                input.bufferViews[accessor.bufferView];
                            positionBuffer = reinterpret_cast<const float*>(
                                &(input.buffers[view.buffer]
                                      .data[accessor.byteOffset +
                                            view.byteOffset]));
                            vertexCount = accessor.count;
                        }
                        if (gltfPrimitive.attributes.find("NORMAL") !=
                            gltfPrimitive.attributes.end())
                        {
                            const auto& accessor =
                                input.accessors[gltfPrimitive.attributes
                                                    .find("NORMAL")
                                                    ->second];
                            const auto& view =
                                input.bufferViews[accessor.bufferView];
                            normalsBuffer = reinterpret_cast<const float*>(
                                &(input.buffers[view.buffer]
                                      .data[accessor.byteOffset +
                                            view.byteOffset]));
                        }
                        if (gltfPrimitive.attributes.find("TEXCOORD_0") !=
                            gltfPrimitive.attributes.end())
                        {
                            const auto& accessor =
                                input.accessors[gltfPrimitive.attributes
                                                    .find("TEXCOORD_0")
                                                    ->second];
                            const auto& view =
                                input.bufferViews[accessor.bufferView];
                            texCoordsBuffer = reinterpret_cast<const float*>(
                                &(input.buffers[view.buffer]
                                      .data[accessor.byteOffset +
                                            view.byteOffset]));
                        }
                        for (size_t v = 0; v < vertexCount; v++)
                        {
                            Vertex vert{};
                            vert.Position = glm::vec4(
                                glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                            vert.Normal = glm::normalize(glm::vec3(
                                normalsBuffer
                                    ? glm::make_vec3(&normalsBuffer[v * 3])
                                    : glm::vec3(0.0f)));
                            vert.Uv =
                                texCoordsBuffer
                                    ? glm::make_vec2(&texCoordsBuffer[v * 2])
                                    : glm::vec3(0.0f);
                            vert.Color = glm::vec3(1.0f);
                            vertices.push_back(vert);
                        }
                    }
                    {
                        const auto& accessor =
                            input.accessors[gltfPrimitive.indices];
                        const auto& bufferView =
                            input.bufferViews[accessor.bufferView];
                        const auto& buffer = input.buffers[bufferView.buffer];

                        indexCount += static_cast<u32>(accessor.count);
                        switch (accessor.componentType)
                        {
                            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                            {
                                const u32* buf = reinterpret_cast<const u32*>(
                                    &buffer.data[accessor.byteOffset +
                                                 bufferView.byteOffset]);
                                for (size_t index = 0; index < accessor.count;
                                     index++)
                                {
                                    indices.push_back(buf[index] + vertexStart);
                                }
                                break;
                            }
                            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                            {
                                const uint16_t* buf =
                                    reinterpret_cast<const uint16_t*>(
                                        &buffer.data[accessor.byteOffset +
                                                     bufferView.byteOffset]);
                                for (size_t index = 0; index < accessor.count;
                                     index++)
                                {
                                    indices.push_back(buf[index] + vertexStart);
                                }
                                break;
                            }
                            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                            {
                                const uint8_t* buf =
                                    reinterpret_cast<const uint8_t*>(
                                        &buffer.data[accessor.byteOffset +
                                                     bufferView.byteOffset]);
                                for (size_t index = 0; index < accessor.count;
                                     index++)
                                {
                                    indices.push_back(buf[index] + vertexStart);
                                }
                                break;
                            }
                            default:
                                FOO_CORE_WARN(
                                    "Index component type {0} not sopperted",
                                    accessor.componentType);
                                return;
                        }
                    }
                    Primitive primitive{};
                    primitive.FirstIndex    = firstIndex;
                    primitive.IndexCount    = indexCount;
                    primitive.MaterialIndex = gltfPrimitive.material;
                    node->mesh.Primitives.push_back(primitive);
                }
            }
            if (parent)
            {
                parent->Children.push_back(node);
            }
            else
            {
                Nodes.push_back(node);
            }
        }
    }  // namespace GLTF

}  // namespace FooGame
