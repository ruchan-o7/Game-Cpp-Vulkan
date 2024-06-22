#include "AssetManager.h"
#include <tiny_obj_loader.h>
#include "../Engine/Geometry/Model.h"
#include "../Engine/Core/VulkanTexture.h"
#include <Log.h>
#include "../Engine/Engine/Backend.h"
#include "ObjLoader.h"
#include "Thread.h"
#include "src/Engine/Core/VulkanBuffer.h"
#include "src/Engine/Engine/Renderer3D.h"
#include "src/Engine/Geometry/Material.h"
#include "src/Engine/Geometry/Mesh.h"
#include <stb_image.h>
#include <tiny_gltf.h>
#include "../Engine/Engine/Renderer3D.h"
#include "../Scene/Asset.h"
#include "../Base.h"
#include "../Core/GltfLoader.h"
#include <cstddef>
#include <memory>
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
    std::unordered_map<ModelName, AssetContainer<std::shared_ptr<Model>>> s_ModelMap;
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

    void AssetManager::LoadGLTFModel(const GltfLoader& loader)
    {
        auto gltfModel = loader.Load();
        DEFER(delete gltfModel);
        if (gltfModel->Name.empty())
        {
            FOO_ENGINE_WARN("Model name empty");
            return;
        }
        auto& asset = s_ModelMap[gltfModel->Name];
        if (asset.Status == AssetStatus::WORKING)
        {
            return;
        }
        asset.Status = AssetStatus::WORKING;

        for (auto& mat : gltfModel->Materials)
        {
            AssetManager::AddMaterial(mat);
        }

        std::string UnnamedImageStr("Unnamed Image");
        for (auto& gltfImage : gltfModel->ImageSources)
        {
            AssetManager::LoadTexture(gltfImage.Name.empty() ? UnnamedImageStr : gltfImage.Name,
                                      gltfImage.ImageBuffer, gltfImage.ImageSize, gltfImage.Width,
                                      gltfImage.Height);
        }
        for (auto& m : gltfModel->Materials)
        {
            InsertMaterial(m);
        }
        auto model = std::make_shared<Model>(std::move(gltfModel->Meshes));
        AssetContainer<std::shared_ptr<Model>> c;
        c.Asset  = model;
        c.Status = AssetStatus::READY;

        s_ModelMap[gltfModel->Name] = c;
        Renderer3D::SubmitModel(model.get());

        return;
    }
    void AssetManager::LoadObjModel(const ObjLoader& loader)
    {
        auto objModel = loader.LoadModel();
        DEFER(objModel.reset(););
        auto& asset  = s_ModelMap[objModel->Name];
        asset.Status = AssetStatus::WORKING;

        for (auto& mat : objModel->Materials)
        {
            AssetManager::AddMaterial(mat);
        }
        for (auto& mat : objModel->Materials)
        {
            auto& texPathStr = mat.PbrMat.BaseColorTexturePath;
            if (texPathStr.empty())
            {
                continue;
            }
            auto texPath = loader.GetPath().parent_path() / texPathStr;
            LoadTexture(texPath.string(), mat.PbrMat.BaseColorTextureName);
        }
        auto modelPtr = std::make_shared<Model>();

        for (size_t i = 0; i < objModel->Meshes.size(); i++)
        {
            modelPtr->m_Meshes.emplace_back(std::move(objModel->Meshes[i]));
        }
        modelPtr->Name = objModel->Name;

        InsertModel(objModel->Name, modelPtr);

        Renderer3D::SubmitModel(objModel->Name);
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

    void AssetManager::InsertMaterial(const Material& m)
    {
        std::lock_guard<std::mutex> lock(g_Material_mutex);
        s_MaterialMap[m.Name] = m;
    }
    void AssetManager::LoadGLTFModelAsync(std::string path, std::string name, bool isGlb)
    {
        // s_pThread->AddTask([=] { AssetManager::LoadGLTFModel(path, name, isGlb); });
    }

    void AssetManager::InsertModel(const std::string& name, std::shared_ptr<Model> m)
    {
        std::lock_guard<std::mutex> lock(g_Model_mutex);
        auto& asset  = s_ModelMap[name];
        asset.Status = AssetStatus::READY;
        asset.Asset  = m;
    }
    AssetContainer<std::shared_ptr<Model>>& AssetManager::GetModelAsset(const std::string& name)
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
                return s_ModelMap[name].Asset;
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
        float data[] = {199, 199, 199, 255};
        LoadTexture("Default Texture", data, sizeof(data), 1, 1);
        g_IsDefaulTextureInitialized = true;
    }

}  // namespace FooGame
