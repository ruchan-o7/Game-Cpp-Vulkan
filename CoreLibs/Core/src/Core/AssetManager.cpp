#include "AssetManager.h"
#include <tiny_obj_loader.h>
#include "../Engine/Geometry/Model.h"
#include "../Engine/Core/VulkanTexture.h"
#include <Log.h>
#include "../Engine/Engine/Backend.h"
#include "Thread.h"
#include "glm/gtc/type_ptr.hpp"
#include "src/Core/File.h"
#include "src/Core/GltfLoader.h"
#include "src/Engine/Core/VulkanBuffer.h"
#include "src/Engine/Engine/Renderer3D.h"
#include "src/Engine/Geometry/Material.h"
#include "src/Engine/Geometry/Mesh.h"
#include <stb_image.h>
#include <tiny_gltf.h>
#include "../Engine/Engine/Renderer3D.h"
#include "src/Log.h"
#include "src/Scene/Asset.h"
#include <cstddef>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
namespace FooGame
{

    std::mutex g_Texture_mutex;
    std::mutex g_Model_mutex;
    std::mutex g_Material_mutex;

    using ModelName = std::string;
    std::unordered_map<ModelName, AssetContainer<Model>> s_ModelMap;
    std::unordered_map<ModelName, AssetContainer<std::shared_ptr<Mesh>>> s_MeshMap;
    std::unordered_map<ModelName, std::shared_ptr<VulkanTexture>> s_TextureMap;

    std::unordered_map<std::string, Material> s_MaterialMap;
    std::unique_ptr<Thread> s_pThread;

    void AssetManager::Init()
    {
        s_pThread = Thread::Create();
    }
    void AssetManager::DeInit()
    {
        s_pThread.reset();
    }

    const std::unordered_map<std::string, Material>& AssetManager::GetAllMaterials()
    {
        return s_MaterialMap;
    }
    void AssetManager::AddMaterial(Material material)
    {
        InsertMaterial(material);
    }

    static bool ReadFile(tinygltf::Model& input, const std::string& path, bool isGlb);

    static void ProcessObjMaterial(const std::vector<tinyobj::material_t>& objMaterials)
    {
        for (const auto& mat : objMaterials)
        {
            Material material;
            material.Name               = mat.name;
            material.fromGlb            = false;
            material.NormalTexture.Name = mat.bump_texname;

            auto& pbr = material.PbrMat;

            pbr.BaseColorFactor[0]   = mat.diffuse[0];
            pbr.BaseColorFactor[1]   = mat.diffuse[1];
            pbr.BaseColorFactor[2]   = mat.diffuse[2];
            pbr.BaseColorFactor[3]   = 1.0;
            pbr.BaseColorTextureName = mat.diffuse_texname;

            pbr.MetallicRoughnessTextureName = mat.roughness_texname;
            pbr.MetallicFactor               = mat.metallic;
            pbr.RoughnessFactor              = mat.roughness;

            AssetManager::AddMaterial(material);
        }
    }

    void AssetManager::LoadObjModel(const std::filesystem::path& path, const std::string& modelName,
                                    std::string materialName)
    {
        if (GetModel(modelName))
        {
            FOO_ENGINE_WARN("Model {0} is already loaded");
            return;
        }
        auto& asset  = s_ModelMap[modelName];
        asset.Status = AssetStatus::WORKING;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Mesh> meshes;

        auto objPath        = path.string();
        auto objBasePath    = path.parent_path();
        auto objBasePathStr = objBasePath.string();

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objPath.c_str(),
                              objBasePathStr.c_str()))
        {
            FOO_ENGINE_ERROR("Model could not loaded {0}", path.string());
            return;
        }
        ProcessObjMaterial(materials);
        for (const auto& [name, mat] : s_MaterialMap)
        {
            auto texPathStr = mat.PbrMat.BaseColorTexturePath;
            if (texPathStr.empty())
            {
                continue;
            }
            auto path = objBasePath / texPathStr;
            LoadTexture(path.string(), mat.PbrMat.BaseColorTextureName);
        }

        for (const auto& shape : shapes)
        {
            Mesh mesh;
            mesh.Name = shape.name;

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
                mesh.m_Vertices.emplace_back(std::move(vertex));
                mesh.m_Indices.push_back(indices.size());
            }
            meshes.emplace_back(std::move(mesh));
        }

        auto modelPtr                = std::make_shared<Model>(std::move(meshes));
        modelPtr->m_Meshes[0].M3Name = materialName;
        enum TEXTURE_INDICES
        {
            ALBEDO    = 0,
            METALIC   = 1,
            ROUGHNESS = 2,
            NORMAL    = 3,
        };
        modelPtr->textureIndices = {ALBEDO, METALIC, ROUGHNESS, NORMAL};
        InsertMap(modelName, modelPtr);

        Renderer3D::SubmitModel(modelName);
    }
    void AssetManager::LoadTexture(const std::string& path, const std::string& name)
    {
        if (GetTexture(name))
        {
            FOO_ENGINE_WARN("Texture {0} is already loaded", name);
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
    size_t g_UnnamedImagesCount = 0;
    void AssetManager::LoadTexture(const std::string& name, void* pixels, size_t size,
                                   int32_t width, int32_t height)
    {
        bool isExists  = false;
        int foundIndex = 0;

        std::string imgName = name;
        if (GetTexture(imgName))
        {
            g_UnnamedImagesCount++;
            imgName += std::to_string(g_UnnamedImagesCount);
        }

        auto* pRenderDevice        = Backend::GetRenderDevice();
        const auto* physicalDevice = pRenderDevice->GetPhysicalDevice();
        VulkanBuffer::BuffDesc stageDesc{};
        stageDesc.pRenderDevice   = pRenderDevice;
        stageDesc.Usage           = Vulkan::BUFFER_USAGE_TRANSFER_SOURCE;
        stageDesc.MemoryFlag      = Vulkan::BUFFER_MEMORY_FLAG_CPU_VISIBLE;
        stageDesc.Name            = std::string("SB Texture: ") + imgName;
        stageDesc.BufferData.Data = pixels;
        stageDesc.BufferData.Size = size;

        VulkanBuffer stageBuffer{stageDesc};
        stageBuffer.MapMemory();
        stageBuffer.UpdateData(pixels, size);
        stageBuffer.UnMapMemory();

        VulkanTexture::CreateInfo ci{};
        ci.pRenderDevice = pRenderDevice;
        ci.MaxAnisotropy = physicalDevice->GetDeviceProperties().limits.maxSamplerAnisotropy;
        ci.Name          = imgName;
        ci.AspectFlags   = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.Format        = VK_FORMAT_R8G8B8A8_SRGB;
        ci.MemoryPropertiesFlags = Vulkan::BUFFER_MEMORY_FLAG_GPU_ONLY;
        ci.Tiling                = VK_IMAGE_TILING_LINEAR;
        ci.UsageFlags            = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ci.Extent                = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        auto* vTexture = new VulkanTexture{ci};
        auto vkImage   = vTexture->GetImage();
        {
            std::lock_guard<std::mutex> lock(g_Texture_mutex);

            Backend::TransitionImageLayout(vkImage.get(), VK_FORMAT_R8G8B8A8_SRGB,
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            Backend::CopyBufferToImage(stageBuffer, *vTexture);
            Backend::TransitionImageLayout(vkImage.get(), VK_FORMAT_R8G8B8A8_SRGB,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        auto texPtr = std::shared_ptr<VulkanTexture>(vTexture);
        InsertTextureToVector(texPtr, imgName);
    }
    void AssetManager::InsertTextureToVector(const std::shared_ptr<VulkanTexture>& pT,
                                             const std::string& name)
    {
        std::lock_guard<std::mutex> lock(g_Texture_mutex);
        s_TextureMap[name] = pT;
    }

    Material AssetManager::GetMaterial(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(g_Material_mutex);
        return s_MaterialMap[name];
    }

    void ProcessGLTFImages(std::vector<tinygltf::Image>& images,
                           std::vector<std::shared_ptr<VulkanTexture>>& vulkanTextures,
                           std::string defaultName = std::string())
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
            auto imageName = gltfImage.name.empty() ? gltfImage.uri : gltfImage.name;
            auto fileName  = File::ExtractFileName(imageName);
            if (fileName.empty())
            {
                AssetManager::LoadTexture(defaultName, (void*)imageBuffer, imageSize,
                                          gltfImage.width, gltfImage.height);
            }
            else
            {
                AssetManager::LoadTexture(fileName, (void*)imageBuffer, imageSize, gltfImage.width,
                                          gltfImage.height);
            }
            auto texture = AssetManager::GetTexture(fileName);
            if (texture)
            {
                vulkanTextures.push_back(texture);
            }
            if (deleteBuffer)
            {
                delete[] imageBuffer;
            }
        }
    }

    static std::vector<Material> ProcessGLTFMaterial(
        const tinygltf::Model& gltfModel, std::string defaultBaseColorTextureName = std::string())
    {
        std::vector<Material> materials;

        const auto& gMats     = gltfModel.materials;
        const auto& gImages   = gltfModel.images;
        const auto& gTextures = gltfModel.textures;  // TODO fix unnamed texture name
        for (auto& m : gMats)
        {
            Material mt;
            mt.fromGlb = true;
            mt.Name    = m.name;
            if (m.pbrMetallicRoughness.baseColorTexture.index != -1)
            {
                auto& texture = gTextures[m.pbrMetallicRoughness.baseColorTexture.index];

                auto gBaseColorTexture = gImages
                    [texture.source];  // gImages[m.pbrMetallicRoughness.baseColorTexture.index];
                auto baseColorTextureName =
                    gBaseColorTexture.name.empty() ? gBaseColorTexture.uri : gBaseColorTexture.name;
                mt.PbrMat.BaseColorTextureName = File::ExtractFileName(baseColorTextureName);

                mt.PbrMat.BaseColorFactor[0] = m.pbrMetallicRoughness.baseColorFactor[0];
                mt.PbrMat.BaseColorFactor[1] = m.pbrMetallicRoughness.baseColorFactor[1];
                mt.PbrMat.BaseColorFactor[2] = m.pbrMetallicRoughness.baseColorFactor[2];
                mt.PbrMat.BaseColorFactor[3] = m.pbrMetallicRoughness.baseColorFactor[3];

                mt.PbrMat.MetallicFactor       = m.pbrMetallicRoughness.metallicFactor;
                mt.PbrMat.RoughnessFactor      = m.pbrMetallicRoughness.roughnessFactor;
                mt.PbrMat.BaseColorTexturePath = gBaseColorTexture.uri;
            }
            if (m.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
            {
                auto& texture = gTextures[m.pbrMetallicRoughness.metallicRoughnessTexture.index];

                auto& metallicRoughness = gImages
                    [texture
                         .source];  //    gImages[m.pbrMetallicRoughness.metallicRoughnessTexture.index];
                mt.PbrMat.MetallicRoughnessTextureName =
                    metallicRoughness.name.empty() ? File::ExtractFileName(metallicRoughness.uri)
                                                   : metallicRoughness.name;
                mt.PbrMat.MetallicRoughnessTexturePath = metallicRoughness.uri;
            }
            if (m.normalTexture.index != -1)
            {
                auto& texture       = gTextures[m.normalTexture.index];
                auto& normalTexture = gImages[texture.source];  // gImages[m.normalTexture.index];
                auto normalTexturename =
                    normalTexture.name.empty() ? normalTexture.uri : normalTexture.name;
                mt.NormalTexture.Name = File::ExtractFileName(normalTexturename);
            }
            materials.push_back(mt);
        }

        return materials;
    }
    void AssetManager::InsertMaterial(const Material& m)
    {
        std::lock_guard<std::mutex> lock(g_Material_mutex);
        s_MaterialMap[m.Name] = m;
    }
    void AssetManager::LoadGLTFModelAsync(std::string path, std::string name, bool isGlb)
    {
        s_pThread->AddTask([=] { AssetManager::LoadGLTFModel(path, name, isGlb); });
    }
    void AssetManager::LoadGLTFModel(std::string path, std::string name, bool isGlb)
    {
        if (name.empty() || path.empty())
        {
            FOO_ENGINE_WARN("Name or path is empty of gltf file");
        }
        auto& asset = s_ModelMap[name];
        if (asset.Status == AssetStatus::READY)
        {
            FOO_ENGINE_WARN("Model {0} is already loaded", name);
            return;
        }
        else if (asset.Status == AssetStatus::WORKING || asset.Status == AssetStatus::FAILED)
        {
            return;
        }
        asset.Status = AssetStatus::WORKING;
        GltfLoader loader{path, name, isGlb};
        auto gltfModel = loader.Load();
        if (gltfModel->Name.empty())
        {
            FOO_ENGINE_ERROR("Can not load gltf model {0}", name);
            delete gltfModel;
            return;
        }
        auto data = (char*)gltfModel->ImageSources[0].ImageBuffer;  // for test delete after impl
        std::string UnnamedImageStr = std::string("Unnamed Image");
        for (auto& gltfImage : gltfModel->ImageSources)
        {
            AssetManager::LoadTexture(gltfImage.Name.empty() ? UnnamedImageStr : gltfImage.Name,
                                      gltfImage.ImageBuffer, gltfImage.ImageSize, gltfImage.Width,
                                      gltfImage.Height);
            // TODO Delete loaded texture buffer data
        }
        for (auto& m : gltfModel->Materials)
        {
            InsertMaterial(m);
        }

        for (auto* mesh : gltfModel->Meshes)
        {
            AssetContainer<std::shared_ptr<Mesh>> container;
            container.Status      = AssetStatus::READY;
            container.Asset       = std::shared_ptr<Mesh>(mesh);
            container.Asset->Name = mesh->Name;
            s_MeshMap[name]       = container;
            auto& asset           = s_MeshMap[name];
            Renderer3D::SubmitMesh(*asset.Asset);
        }

        delete gltfModel;
        return;
    }
    void AssetManager::InsertMap(const std::string& name, std::shared_ptr<Model> m)
    {
        std::lock_guard<std::mutex> lock(g_Model_mutex);
        auto& asset  = s_ModelMap[name];
        asset.Status = AssetStatus::READY;
    }
    AssetContainer<Model>& AssetManager::GetModelAsset(const std::string& name)
    {
        return s_ModelMap[name];
    }
    AssetContainer<std::shared_ptr<Mesh>>& AssetManager::GetMeshAsset(const std::string& name)
    {
        return s_MeshMap[name];
    }

    std::shared_ptr<Model> AssetManager::GetModel(const std::string& name)
    {
        if (s_ModelMap.find(name) != s_ModelMap.end())
        {
            auto& asset = s_ModelMap[name];
            if (asset.Status == AssetStatus::READY)
            {
                // return s_ModelMap[name].Asset;
            }
            else if (asset.Status == AssetStatus::WORKING)
            {
                return nullptr;
            }
        }
        return nullptr;
    }
    std::shared_ptr<VulkanTexture> AssetManager::GetTexture(const std::string& name)
    {
        if (name.empty())
        {
            FOO_ENGINE_WARN("Texture name can not be empty");
            return nullptr;
        }

        return s_TextureMap[name];
    }
    bool g_IsDefaulTextureInitialized = false;
    std::shared_ptr<VulkanTexture> AssetManager::GetDefaultTexture()
    {
        if (!g_IsDefaulTextureInitialized)
        {
            throw std::runtime_error("Default texture not initilized");
        }
        return GetTexture("Default Texture");
    }

    void AssetManager::CreateDefaultTexture()
    {
        float data[] = {125, 125, 125, 125};
        LoadTexture("Default Texture", data, sizeof(data), 1, 1);
        g_IsDefaulTextureInitialized = true;
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
