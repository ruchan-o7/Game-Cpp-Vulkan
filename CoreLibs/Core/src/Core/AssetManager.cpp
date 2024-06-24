#include "AssetManager.h"
#include <tiny_obj_loader.h>
#include "../Engine/Geometry/Model.h"
#include "../Engine/Core/VulkanTexture.h"
#include <Log.h>
#include "../Engine/Engine/Backend.h"
#include "ObjLoader.h"
#include "Thread.h"
#include "../Engine/Core/Types.h"
#include "../Engine/Core/VulkanBuffer.h"
#include "../Engine/Engine/Renderer3D.h"
#include "../Engine/Geometry/Material.h"
#include "../Engine/Geometry/Mesh.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include <tiny_gltf.h>
#include "../Engine/Engine/Renderer3D.h"
#include "../Scene/Asset.h"
#include "../Base.h"
#include "../Core/GltfLoader.h"
#include "../Scene/AssetSerializer.h"
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
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

    std::unordered_map<std::string, Material>& AssetManager::GetAllMaterials()
    {
        return s_MaterialMap;
    }
    void AssetManager::AddMaterial(Material material)
    {
        InsertMaterial(material);
    }

    const std::unordered_map<std::string, std::shared_ptr<VulkanTexture>>&
    AssetManager::GetAllImages()
    {
        return s_TextureMap;
    }
    void AssetManager::LoadModel(const Asset::FModel& fmodel)
    {
        auto& asset = s_ModelMap[fmodel.Name];
        if (asset.Status == AssetStatus::WORKING)
        {
            return;
        }
        asset.Status = AssetStatus::WORKING;
        List<Mesh> meshes;
        for (auto& m : fmodel.Meshes)
        {
            Mesh mesh;
            mesh.M3Name = m.MaterialName;
            mesh.m_Vertices.reserve(m.VertexCount);
            for (size_t i = 0; i < m.VertexCount * 11; i += 11)
            {
                Vertex v;
                v.Position.x = m.Vertices[i];
                v.Position.y = m.Vertices[i + 1];
                v.Position.z = m.Vertices[i + 2];

                v.Normal.x = m.Vertices[i + 3];
                v.Normal.y = m.Vertices[i + 4];
                v.Normal.z = m.Vertices[i + 5];

                v.Color.x = m.Vertices[i + 6];
                v.Color.y = m.Vertices[i + 7];
                v.Color.z = m.Vertices[i + 8];

                v.TexCoord.x = m.Vertices[i + 9];
                v.TexCoord.y = m.Vertices[i + 10];
                mesh.m_Vertices.emplace_back(std::move(v));
            }
            mesh.m_Indices = std::move(m.Indices);
            meshes.emplace_back(std::move(mesh));
        }

        auto model   = std::make_shared<Model>(std::move(meshes));
        model->Name  = fmodel.Name;
        asset.Asset  = model;
        asset.Status = AssetStatus::READY;
        Renderer3D::SubmitModel(model.get());
    }

    void AssetManager::LoadGLTFModel(const GltfLoader& loader)
    {
        auto gltfModel = loader.Load();
        // DEFER(delete gltfModel);
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
                                      gltfImage.Height, gltfImage.ComponentCount);
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
        delete gltfModel;
        return;
    }
    void AssetManager::LoadObjModel(Unique<ObjModel> objModel)
    {
        auto& asset  = s_ModelMap[objModel->Name];
        asset.Status = AssetStatus::WORKING;

        if (objModel->Materials.empty())
        {
            for (auto& mesh : objModel->Meshes)
            {
                mesh.M3Name = DEFAULT_MATERIAL_NAME;
            }
        }
        else
        {
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
                auto texPath =
                    mat.PbrMat
                        .BaseColorTexturePath;  // loader.GetPath().parent_path() / texPathStr;
                LoadTexture(texPath, mat.PbrMat.BaseColorTextureName);
            }
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
    {  //! extension related data reading is cumborsome from .fimg refactor it
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
        Defer d{[&] { stbi_image_free(pixels); }};
        if (!pixels)
        {
            FOO_ENGINE_ERROR("Coult not load image : {0}", path);
            return;
        }

        Asset::FImage fimage;
        fimage.Name         = name;
        fimage.Size         = imageSize;
        fimage.Width        = texWidth;
        fimage.Height       = texHeight;
        fimage.ChannelCount = STBI_rgb_alpha;
        fimage.Format       = Asset::TextureFormat::RGBA8;

        auto images    = File::GetImagesPath();
        auto assetFile = images / name;
        if (!std::filesystem::exists(assetFile))
        {
            assetFile.replace_extension(".fimgbuff");
            fimage.BufferPath = assetFile.string();
            stbi_write_bmp(assetFile.string().c_str(), fimage.Width, fimage.Height, STBI_rgb_alpha,
                           pixels);
            ImageSerializer is;
            auto j = is.Serialize(fimage);
            assetFile.replace_extension(".fimg");
            std::ofstream o{assetFile};
            DEFER(o.close());
            o << std::setw(4) << j << std::endl;
        }
        AssetManager::LoadTexture(fimage);
    }
    void AssetManager::LoadTexture(Asset::FImage& fimage)
    {
        int x, y, channels;
        stbi_uc* pixels = stbi_load(fimage.BufferPath.c_str(), &x, &y, &channels, STBI_rgb_alpha);
        DEFER(stbi_image_free(pixels));
        AssetManager::LoadTexture(fimage.Name, pixels, fimage.Size, x, y,
                                  /*texChannels*/ STBI_rgb_alpha);
    }

    size_t g_UnnamedImagesCount = 0;
    void AssetManager::LoadTexture(const std::string& name, void* pixels, size_t size,
                                   int32_t width, int32_t height, int32_t channelCount)
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
        if (channelCount == 4)
        {
            ci.Format = VK_FORMAT_R8G8B8A8_SRGB;
        }
        else
        {
            ci.Format = VK_FORMAT_R8G8B8_SRGB;
        }
        ci.MemoryPropertiesFlags = Vulkan::BUFFER_MEMORY_FLAG_GPU_ONLY;
        ci.Tiling                = VK_IMAGE_TILING_LINEAR;
        ci.UsageFlags            = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        ci.Extent       = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        ci.Size         = size;
        ci.Width        = width;
        ci.Height       = height;
        ci.ChannelCount = channelCount;

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

    Material* AssetManager::GetMaterial(const std::string& name)
    {
        if (s_MaterialMap.find(name) != s_MaterialMap.end())
        {
            return &s_MaterialMap[name];
        }
        return nullptr;
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
    Material* AssetManager::GetDefaultMaterial()
    {
        return &s_MaterialMap[DEFAULT_MATERIAL_NAME];
    }

    std::shared_ptr<VulkanTexture> AssetManager::GetDefaultTexture()
    {
        if (!g_IsDefaulTextureInitialized)
        {
            throw std::runtime_error("Default texture not initilized");
        }
        return GetTexture(DEFAULT_TEXTURE_NAME);
    }

    void AssetManager::CreateDefaultTexture()
    {
        float data[] = {199, 199, 199, 255};
        LoadTexture(DEFAULT_TEXTURE_NAME, data, sizeof(data), 1, 1, 4);
        Material m;
        m.Name                        = DEFAULT_MATERIAL_NAME;
        m.PbrMat.BaseColorTextureName = DEFAULT_TEXTURE_NAME;
        InsertMaterial(m);
        g_IsDefaulTextureInitialized = true;
    }

}  // namespace FooGame
