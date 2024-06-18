#include "AssetManager.h"

#include <tiny_obj_loader.h>
#include "../Engine/Geometry/Model.h"
#include "../Engine/Core/VulkanTexture.h"
#include <Log.h>
#include "../Engine/Engine/Backend.h"
#include "glm/gtc/type_ptr.hpp"
#include "src/Engine/Core/VulkanBuffer.h"
#include <stb_image.h>
#include <tiny_gltf.h>
#include <cstddef>
namespace FooGame
{
    using ModelName = std::string;
    std::unordered_map<ModelName, std::shared_ptr<Model>> s_ModelMap;
    std::unordered_map<ModelName, std::shared_ptr<VulkanTexture>> s_TextureMap;
    std::unordered_map<std::string, Material2> s_MaterialMap;
    std::unordered_map<std::string, Material2> AssetManager::AllMaterials()
    {
        return s_MaterialMap;
    }

    static bool ReadFile(tinygltf::TinyGLTF& context, tinygltf::Model& input,
                         const std::string& path, bool isGlb);
    void AssetManager::LoadObjModel(const std::string& path, const std::string& modelName)
    {
        if (s_ModelMap[modelName])
        {
            FOO_ENGINE_WARN("Model {0} is already loaded");
            return;
        }

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Mesh> meshes;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
        {
            FOO_ENGINE_ERROR("Model could not loaded {0}", path);
            return;
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
                if (index.texcoord_index >= 0)
                {
                    vertex.TexCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }
                else
                {
                    vertex.TexCoord = {1.0f, 1.0f};
                }
                vertex.Color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };  // im not sure
                if (index.normal_index >= 0)
                {
                    vertex.Normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };  // im not sure
                }
                else
                {
                    vertex.Normal = {1.0f, 1.0f, 1.0f};
                }
                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
            meshes.push_back(std::move(Mesh{std::move(vertices), std::move(indices)}));
        }
        s_ModelMap[modelName] = std::make_shared<Model>(std::move(meshes));
    }
    void AssetManager::LoadTexture(const std::string& path, const std::string& name)
    {
        if (s_TextureMap[name])
        {
            FOO_ENGINE_WARN("Texture {0} is already loaded");
            return;
        }

        auto* pRenderDevice = Backend::GetRenderDevice();

        int32_t texWidth, texHeight, texChannels;
        stbi_uc* pixels =
            stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        size_t imageSize = texWidth * texHeight * 4;
        if (!pixels)
        {
            FOO_ENGINE_ERROR("Coult not load image : {0}", path);
            return;
        }
        AssetManager::LoadTexture(name, pixels, imageSize, texWidth, texHeight);
        stbi_image_free(pixels);
    }
    void AssetManager::LoadTexture(const std::string& name, void* pixels, size_t size,
                                   int32_t width, int32_t height)
    {
        auto* pRenderDevice        = Backend::GetRenderDevice();
        const auto* physicalDevice = pRenderDevice->GetPhysicalDevice();

        VulkanBuffer::BuffDesc stageDesc{};
        stageDesc.pRenderDevice   = pRenderDevice;
        stageDesc.Usage           = Vulkan::BUFFER_USAGE_TRANSFER_SOURCE;
        stageDesc.MemoryFlag      = Vulkan::BUFFER_MEMORY_FLAG_CPU_VISIBLE;
        auto bufferName           = std::string("Stage buffer for texture: ") + name;
        stageDesc.Name            = bufferName.c_str();
        stageDesc.BufferData.Data = pixels;
        stageDesc.BufferData.Size = size;

        VulkanBuffer stageBuffer{stageDesc};
        stageBuffer.MapMemory();
        stageBuffer.UpdateData(pixels, size);
        stageBuffer.UnMapMemory();

        VulkanTexture::CreateInfo ci{};
        ci.pRenderDevice = pRenderDevice;
        ci.MaxAnisotropy = physicalDevice->GetDeviceProperties().limits.maxSamplerAnisotropy;
        ci.Name          = name.c_str();
        ci.AspectFlags   = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.Format        = VK_FORMAT_R8G8B8A8_SRGB;
        ci.MemoryPropertiesFlags = Vulkan::BUFFER_MEMORY_FLAG_GPU_ONLY;
        ci.Tiling                = VK_IMAGE_TILING_LINEAR;
        ci.UsageFlags            = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ci.Extent                = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        auto* vTexture = new VulkanTexture{ci};
        auto vkImage   = vTexture->GetImage();

        Backend::TransitionImageLayout(vkImage.get(), VK_FORMAT_R8G8B8A8_SRGB,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        Backend::CopyBufferToImage(stageBuffer, *vTexture);
        Backend::TransitionImageLayout(vkImage.get(), VK_FORMAT_R8G8B8A8_SRGB,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        s_TextureMap[name] = std::shared_ptr<VulkanTexture>(vTexture);
    }
    void AssetManager::AddMaterial(const Material2& material)
    {
        if (!s_MaterialMap[material.Name].Name.empty())
        {
            FOO_ENGINE_WARN("Material {0} is already loaded, skipping", material.Name);
            return;
        }
        s_MaterialMap[material.Name] = material;
    }
    Material2 AssetManager::GetMaterial(const std::string& name)
    {
        return s_MaterialMap[name];
    }
    void AssetManager::LoadGLTFModel(const std::string& path, const std::string& name, bool isGlb)
    {
        if (s_ModelMap[name])
        {
            FOO_ENGINE_WARN("Model {0} is already loaded", name);
            return;
        }

        tinygltf::Model gltfInput;
        tinygltf::TinyGLTF gltfContext;
        std::vector<Mesh> meshes;
        std::vector<std::shared_ptr<VulkanTexture>> vulkanTextures;
        std::vector<uint32_t> textureIndices;

        if (!ReadFile(gltfContext, gltfInput, path, isGlb))
        {
            return;
        }
        for (size_t i = 0; i < gltfInput.scenes.size(); i++)
        {
            // IMAGE
            for (auto& gltfImage : gltfInput.images)
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
                LoadTexture(gltfImage.name, (void*)imageBuffer, imageSize, gltfImage.width,
                            gltfImage.height);
                vulkanTextures.push_back(GetTexture(gltfImage.name));
                if (deleteBuffer)
                {
                    delete[] imageBuffer;
                }
            }
            // TEXTURE
            {
                for (size_t i = 0; i < gltfInput.textures.size(); i++)
                {
                    textureIndices.push_back(gltfInput.textures[i].source);
                }
            }

            // MESH
            const auto& node = gltfInput.nodes[i];
            {
                const auto meshIndex = node.mesh;
                if (meshIndex <= -1)
                {
                    continue;
                }
                const auto mesh = gltfInput.meshes[meshIndex];
                FOO_CORE_TRACE("Mesh : {0} will be proceed", mesh.name);
                for (const auto& primitive : mesh.primitives)
                {
                    FooGame::Mesh tempMesh{};
                    Material materialData{};
                    uint32_t firstIndex       = static_cast<uint32_t>(tempMesh.m_Indices.size());
                    uint32_t vertexStart      = static_cast<uint32_t>(tempMesh.m_Vertices.size());
                    uint32_t indexCount       = 0;
                    const auto indicesIndex   = primitive.indices;
                    const auto materialIndex  = primitive.material;
                    const auto positionsIndex = primitive.attributes.at("POSITION");
                    const auto uvsIndex       = primitive.attributes.at("TEXCOORD_0");
                    const auto normalsIndex   = primitive.attributes.at("NORMAL");

                    const auto material = gltfInput.materials[materialIndex];
                    materialData.BaseColorTextureIndex =
                        material.pbrMetallicRoughness.baseColorTexture.index;
                    materialData.MetallicRoughnessIndex =
                        material.pbrMetallicRoughness.metallicRoughnessTexture.index;
                    materialData.NormalTextureIndex = material.normalTexture.index;
                    materialData.Name               = material.name;
                    tempMesh.materialData           = materialData;

                    const auto indicesAccessor   = gltfInput.accessors[indicesIndex];
                    const auto positionsAccessor = gltfInput.accessors[positionsIndex];
                    const auto uvsAccessor       = gltfInput.accessors[uvsIndex];
                    const auto normalsAccessor   = gltfInput.accessors[normalsIndex];

                    const auto& indicesBufferView =
                        gltfInput.bufferViews[indicesAccessor.bufferView];
                    const auto& positionBV = gltfInput.bufferViews[positionsAccessor.bufferView];
                    const auto& uvsBV      = gltfInput.bufferViews[uvsAccessor.bufferView];
                    const auto& normalsBV  = gltfInput.bufferViews[normalsAccessor.bufferView];
                    const float* positionsBuffer = reinterpret_cast<const float*>(
                        &(gltfInput.buffers[positionBV.buffer]
                              .data[positionsAccessor.byteOffset + positionBV.byteOffset]));

                    const float* normalsBuffer = reinterpret_cast<const float*>(
                        &(gltfInput.buffers[normalsBV.buffer]
                              .data[normalsAccessor.byteOffset + normalsBV.byteOffset]));

                    const float* uvsBuffer = reinterpret_cast<const float*>(
                        &(gltfInput.buffers[uvsBV.buffer]
                              .data[uvsAccessor.byteOffset + uvsBV.byteOffset]));

                    const float* indicesBuffer = reinterpret_cast<const float*>(
                        &(gltfInput.buffers[indicesBufferView.buffer]
                              .data[indicesAccessor.byteOffset + indicesBufferView.byteOffset]));

                    tempMesh.m_Vertices.reserve(positionsAccessor.count);
                    for (size_t w = 0; w < positionsAccessor.count; w++)
                    {
                        FooGame::Vertex v{};

                        v.Position = glm::vec4(glm::make_vec3(&positionsBuffer[w * 3]), 1.0f);
                        v.Normal   = glm::normalize(
                            glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[w * 3])
                                                      : glm::vec3(0.0f)));
                        v.TexCoord = uvsBuffer ? glm::vec2(glm::make_vec2(&uvsBuffer[w * 2]))
                                               : glm::vec2(0.0f);
                        tempMesh.m_Vertices.push_back(v);
                    }
                    {
                        const auto indexCount = static_cast<uint32_t>(indicesAccessor.count);
                        switch (indicesAccessor.componentType)
                        {
                            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                            {
                                const auto& indicesBuff =
                                    gltfInput.buffers[indicesBufferView.buffer];
                                const uint32_t* buf = reinterpret_cast<const uint32_t*>(
                                    &indicesBuff.data[indicesAccessor.byteOffset +
                                                      indicesBufferView.byteOffset]);
                                for (size_t index = 0; index < indicesAccessor.count; index++)
                                {
                                    tempMesh.m_Indices.push_back(buf[index] + vertexStart);
                                }
                                break;
                            }
                            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                            {
                                const auto& indicesBuff =
                                    gltfInput.buffers[indicesBufferView.buffer];
                                const uint16_t* buf = reinterpret_cast<const uint16_t*>(
                                    &indicesBuff.data[indicesAccessor.byteOffset +
                                                      indicesBufferView.byteOffset]);
                                // indexCount +=
                                // static_cast<uint32_t>(accessor.count);
                                for (size_t index = 0; index < indicesAccessor.count; index++)
                                {
                                    tempMesh.m_Indices.push_back(buf[index] + vertexStart);
                                }
                                break;
                            }
                            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                            {
                                const auto& indicesBuff =
                                    gltfInput.buffers[indicesBufferView.buffer];
                                const uint8_t* buf = reinterpret_cast<const uint8_t*>(
                                    &indicesBuff.data[indicesAccessor.byteOffset +
                                                      indicesBufferView.byteOffset]);
                                for (size_t index = 0; index < indicesAccessor.count; index++)
                                {
                                    tempMesh.m_Indices.push_back(buf[index] + vertexStart);
                                }
                                break;
                            }
                            default:
                                FOO_CORE_WARN(
                                    "Index component type {0} not "
                                    "sopperted",
                                    indicesAccessor.componentType);
                                break;
                        }
                    }
                    meshes.emplace_back(std::move(tempMesh));
                }
            }
        }
        auto model = std::make_shared<Model>(std::move(meshes));
        // model->Textures       = vulkanTextures;
        model->textureIndices = textureIndices;
        s_ModelMap[name]      = model;
    }
    std::shared_ptr<Model> AssetManager::GetModel(const std::string& name)
    {
        if (s_ModelMap.find(name) != s_ModelMap.end())
        {
            return s_ModelMap[name];
        }
        return nullptr;
    }
    std::shared_ptr<VulkanTexture> AssetManager::GetTexture(const std::string& name)
    {
        return s_TextureMap[name];
    }

    bool ReadFile(tinygltf::TinyGLTF& context, tinygltf::Model& input, const std::string& path,
                  bool isGlb)
    {
        std::string error, warning;
        bool ret;
        if (isGlb)
        {
            ret = context.LoadBinaryFromFile(&input, &error, &warning, path);
        }
        else
        {
            ret = context.LoadASCIIFromFile(&input, &error, &warning, path);
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
