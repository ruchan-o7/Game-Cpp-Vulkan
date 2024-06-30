#include "GltfLoader.h"
#include <tiny_gltf.h>
#include "File.h"
#include <Log.h>
#include <cstddef>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "glm/ext/matrix_transform.hpp"
#include "src/Engine/Geometry/Vertex.h"

namespace FooGame
{
    // TODO: Delete alloacted buffer in destructor
    static void ProcessGLTFImages(List<tinygltf::Image>& images, List<GltfImageSource>& out);

    static void ProcessGLTFMaterial(const List<tinygltf::Material>& gltfModel,
                                    List<GltfMaterialSource>& out);

    bool ReadFile(tinygltf::Model& input, const std::string& path, bool isGlb);
    GltfLoader::GltfLoader(const std::filesystem::path& path, bool isGlb)
        : m_Path(path), m_Name(path.filename().string()), m_IsGlb(isGlb)
    {
        if (m_Name.empty() || m_Path.empty())
        {
            FOO_ENGINE_WARN("Name or path is empty, name: {0}, path {1}", m_Name, m_Path.string());
            m_EligibleToLoad = false;
        }
        else
        {
            m_EligibleToLoad = true;
        }
    }
    void ProcessNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent,
                     List<Vertex>& vertices, List<u32>& indices, List<Node*>& nodes)
    {
        auto* node   = new Node();
        node->Matrix = glm::mat4(1.0f);
        node->Parent = parent;
        if (inputNode.translation.size() == 3)
        {
            node->Matrix = glm::translate(node->Matrix,
                                          glm::vec3(glm::make_vec3(inputNode.translation.data())));
        }
        if (inputNode.rotation.size() == 4)
        {
            glm::quat q   = glm::make_quat(inputNode.rotation.data());
            node->Matrix *= glm::mat4(q);
        }
        if (inputNode.scale.size() == 3)
        {
            node->Matrix =
                glm::scale(node->Matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
        }
        if (inputNode.matrix.size() == 16)
        {
            node->Matrix = glm::make_mat4x4(inputNode.matrix.data());
        };

        if (inputNode.children.size() > 0)
        {
            for (size_t i = 0; i < inputNode.children.size(); i++)
            {
                ProcessNode(input.nodes[inputNode.children[i]], input, node, vertices, indices,
                            nodes);
            }
        }

        if (inputNode.mesh > -1)
        {
            const auto& mesh = input.meshes[inputNode.mesh];
            for (size_t i = 0; i < mesh.primitives.size(); i++)
            {
                const auto& glTFPrimitive = mesh.primitives[i];
                u32 firstIndex            = static_cast<u32>(vertices.size());
                u32 vertexStart           = static_cast<u32>(indices.size());
                u32 indexCount            = 0;
                // Vertices
                {
                    const float* positionBuffer  = nullptr;
                    const float* normalsBuffer   = nullptr;
                    const float* texCoordsBuffer = nullptr;

                    size_t vertexCount = 0;

                    if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end())
                    {
                        const auto& accessor =
                            input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                        const auto& view = input.bufferViews[accessor.bufferView];
                        positionBuffer   = reinterpret_cast<const float*>(
                            &(input.buffers[view.buffer]
                                  .data[accessor.byteOffset + view.byteOffset]));

                        vertexCount = accessor.count;
                    }

                    if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end())
                    {
                        const auto& accessor =
                            input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                        const auto& view = input.bufferViews[accessor.bufferView];
                        normalsBuffer    = reinterpret_cast<const float*>(
                            &(input.buffers[view.buffer]
                                  .data[accessor.byteOffset + view.byteOffset]));
                    }
                    if (glTFPrimitive.attributes.find("TEXCOORD_0") !=
                        glTFPrimitive.attributes.end())
                    {
                        const auto& accessor =
                            input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                        const auto& view = input.bufferViews[accessor.bufferView];
                        texCoordsBuffer  = reinterpret_cast<const float*>(
                            &(input.buffers[view.buffer]
                                  .data[accessor.byteOffset + view.byteOffset]));
                    }

                    for (size_t v = 0; v < vertexCount; v++)
                    {
                        Vertex vert{};
                        vert.Position = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                        vert.Normal   = glm::normalize(
                            glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3])
                                                      : glm::vec3(0.0f)));
                        vert.TexCoord = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2])
                                                        : glm::vec3(0.0f);
                        vert.Color    = glm::vec3(1.0f);
                        vertices.push_back(vert);
                    }
                }
                // INDICES
                {
                    const auto& accessor   = input.accessors[glTFPrimitive.indices];
                    const auto& bufferView = input.bufferViews[accessor.bufferView];
                    const auto& buffer     = input.buffers[bufferView.buffer];

                    indexCount += static_cast<u32>(accessor.count);

                    switch (accessor.componentType)
                    {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                        {
                            const u32* buf = reinterpret_cast<const u32*>(
                                &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++)
                            {
                                indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                        {
                            const uint16_t* buf = reinterpret_cast<const uint16_t*>(
                                &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++)
                            {
                                indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                        {
                            const u8* buf = reinterpret_cast<const u8*>(
                                &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++)
                            {
                                indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        default:
                            FOO_CORE_ERROR("Index component type: {0} not supported!",
                                           accessor.componentType);
                            return;
                    }
                }
                GltfPrimitive primitive{};
                primitive.firstIndex    = firstIndex;
                primitive.indexCount    = indexCount;
                primitive.materialIndex = glTFPrimitive.material;
                node->Mesh.Name         = mesh.name;
                node->Mesh.primitives.push_back(primitive);
            }
        }
        if (parent)
        {
            parent->Children.push_back(node);
        }
        else
        {
            nodes.push_back(node);
        }
    }
    Unique<GltfModel> GltfLoader::Load() const
    {
        if (!m_EligibleToLoad)
        {
            return nullptr;
        }
        tinygltf::Model gltfInput;
        GltfModel* model = new GltfModel;
        model->Name      = File::ExtractFileName(m_Path);

        auto readRes = ReadFile(gltfInput, m_Path.string(), m_IsGlb);

        if (!readRes)
        {
            FOO_ENGINE_ERROR("Could not load gltf model.");
            return nullptr;
        }
        ProcessGLTFImages(gltfInput.images, model->ImageSources);
        ProcessGLTFMaterial(gltfInput.materials, model->Materials);

        model->Textures.resize(gltfInput.textures.size());
        for (size_t i = 0; i < model->Textures.size(); i++)
        {
            model->Textures[i] = gltfInput.textures[i].source;
        }

        const auto& scene = gltfInput.scenes[0];

        for (size_t i = 0; i < scene.nodes.size(); i++)
        {
            const auto node = gltfInput.nodes[scene.nodes[i]];
            ProcessNode(node, gltfInput, nullptr, model->Vertices, model->Indices, model->Nodes);
        }
        return Unique<GltfModel>(model);
    }

    void ProcessGLTFImages(List<tinygltf::Image>& images, List<GltfImageSource>& out)
    {
        for (auto& gltfImage : images)
        {
            unsigned char* imageBuffer = nullptr;
            size_t imageSize           = 0;
            bool deleteBuffer          = false;

            if (gltfImage.component == 3)
            {
                imageSize           = gltfImage.width * gltfImage.height * 4;
                imageBuffer         = new unsigned char[imageSize];
                unsigned char* rgba = imageBuffer;
                unsigned char* rgb  = &gltfImage.image[0];
                for (size_t j = 0; j < gltfImage.width * gltfImage.height; j++)
                {
                    memcpy(rgba, rgb, sizeof(unsigned char) * 3);
                    rgba += 4;
                    rgb  += 3;
                }
                deleteBuffer = true;
            }
            else
            {
                imageBuffer = &gltfImage.image[0];
                imageSize   = gltfImage.image.size();
            }
            GltfImageSource source{};

            source.ImageBuffer = new unsigned char[imageSize];
            memcpy(source.ImageBuffer, (void*)imageBuffer, imageSize);

            source.ImageSize      = imageSize;
            source.Width          = gltfImage.width;
            source.Height         = gltfImage.height;
            source.ComponentCount = gltfImage.component;

            auto imageName = gltfImage.name.empty() ? gltfImage.uri : gltfImage.name;
            auto fileName  = File::ExtractFileName(imageName);
            if (fileName.empty())
            {
                source.Name = "UnNamed Image";
            }
            else
            {
                source.Name = fileName;
            }
            out.emplace_back(std::move(source));
        }
    }
    void ProcessGLTFMaterial(const List<tinygltf::Material>& input, List<GltfMaterialSource>& out)
    {
        out.resize(input.size());
        for (size_t i = 0; i < input.size(); i++)
        {
            auto& gltfMaterial = input[i];
            out[i].Name        = input[i].name;

            if (gltfMaterial.values.find("baseColorFactor") != gltfMaterial.values.end())
            {
                out[i].BaseColorFactor =
                    glm::make_vec4(gltfMaterial.values.at("baseColorFactor").ColorFactor().data());
            }
            if (gltfMaterial.values.find("baseColorTexture") != gltfMaterial.values.end())
            {
                out[i].BaseColorTexIndex =
                    gltfMaterial.values.at("baseColorTexture").TextureIndex();
            }
        }
#if 0 
        const auto& gMats     = gltfModel.materials;
        const auto& gImages   = gltfModel.images;
        const auto& gTextures = gltfModel.textures;
        size_t index          = 0;
        auto sceneName        = gltfModel.scenes[0].name + "_";
        for (auto& m : gMats)
        {
            GltfMaterialSource mt;
            mt.Name = m.name;
            if (m.pbrMetallicRoughness.baseColorTexture.index != -1)
            {
                auto& texture = gTextures[m.pbrMetallicRoughness.baseColorTexture.index];

                auto gBaseColorTexture = gImages[texture.source];
                auto baseColorTextureName =
                    gBaseColorTexture.name.empty() ? gBaseColorTexture.uri : gBaseColorTexture.name;
                mt.BaseColorTextureName = File::ExtractFileName(baseColorTextureName);
                if (mt.BaseColorTextureName.empty())
                {
                    mt.BaseColorTextureName = std::string(sceneName + std::to_string(index));
                    index++;
                }
                mt.BaseColorTextureFactor[0] = m.pbrMetallicRoughness.baseColorFactor[0];
                mt.BaseColorTextureFactor[1] = m.pbrMetallicRoughness.baseColorFactor[1];
                mt.BaseColorTextureFactor[2] = m.pbrMetallicRoughness.baseColorFactor[2];
                mt.BaseColorTextureFactor[3] = m.pbrMetallicRoughness.baseColorFactor[3];

                mt.MetallicFactor  = m.pbrMetallicRoughness.metallicFactor;
                mt.RoughnessFactor = m.pbrMetallicRoughness.roughnessFactor;
            }
            if (m.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
            {
                auto& texture = gTextures[m.pbrMetallicRoughness.metallicRoughnessTexture.index];

                auto& metallicRoughness = gImages[texture.source];
                mt.MetallicTextureName  = metallicRoughness.name.empty()
                                              ? File::ExtractFileName(metallicRoughness.uri)
                                              : metallicRoughness.name;

                if (mt.MetallicTextureName.empty())
                {
                    mt.MetallicTextureName = std::string(sceneName + std::to_string(index));
                    index++;
                }
            }
            if (m.normalTexture.index != -1)
            {
                auto& texture       = gTextures[m.normalTexture.index];
                auto& normalTexture = gImages[texture.source];
                auto normalTexturename =
                    normalTexture.name.empty() ? normalTexture.uri : normalTexture.name;
                if (normalTexturename.empty())
                {
                    normalTexturename = std::string(sceneName + std::to_string(index));
                    index++;
                }

                mt.NormalTextureName = File::ExtractFileName(normalTexturename);
            }
            out.push_back(mt);
        }
#endif
    }
    bool ReadFile(tinygltf::Model& input, const std::string& path, bool isGlb)
    {
        std::string error, warning;
        tinygltf::TinyGLTF gltfContext;

        bool ret;
        if (isGlb)
        {
            ret = gltfContext.LoadBinaryFromFile(&input, &error, &warning, path);
        }
        else
        {
            ret = gltfContext.LoadASCIIFromFile(&input, &error, &warning, path);
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
            FOO_CORE_ERROR("Failed to parse glTF : {0}", path);
        }
        return ret;
    }
}  // namespace FooGame
