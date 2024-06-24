#include "AssetManager.h"
#include <tiny_obj_loader.h>
#include <Log.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <tiny_gltf.h>
#include <pch.h>
#include <cstddef>
#include <filesystem>
#include <utility>
#include "../Engine/Geometry/Model.h"
#include "../Engine/Core/VulkanTexture.h"
#include "../Engine/Engine/Backend.h"
#include "../Base.h"
#include "ObjLoader.h"
#include "Thread.h"
#include "../Engine/Core/Types.h"
#include "../Engine/Core/VulkanBuffer.h"
#include "../Engine/Engine/Renderer3D.h"
#include "../Engine/Geometry/Mesh.h"
#include "../Engine/Engine/Renderer3D.h"
#include "../Scene/Asset.h"
#include "../Base.h"
#include "../Core/GltfLoader.h"
#include "../Scene/AssetSerializer.h"
#include "../Base.h"
#include "../Core/File.h"
namespace FooGame
{

    std::mutex g_Texture_mutex;
    std::mutex g_Model_mutex;
    std::mutex g_Material_mutex;

    using ModelName = std::string;
    std::unordered_map<ModelName, AssetContainer<std::shared_ptr<Model>>> s_ModelMap;
    std::unordered_map<ModelName, AssetContainer<std::shared_ptr<Mesh>>> s_MeshMap;
    std::unordered_map<ModelName, std::shared_ptr<VulkanTexture>> s_TextureMap;

    std::unordered_map<std::string, Asset::FMaterial> s_MaterialMap;
    std::unique_ptr<Thread> s_pThread;

    void AssetManager::Init()
    {
        s_pThread = Thread::Create();
        CreateDefaultTexture();
    }
    void AssetManager::DeInit()
    {
        s_pThread.reset();
    }

    std::unordered_map<std::string, Asset::FMaterial>& AssetManager::GetAllMaterials()
    {
        return s_MaterialMap;
    }
    void AssetManager::AddMaterial(Asset::FMaterial material)
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

    void AssetManager::LoadGLTFModel(GltfModel& gltfModel)
    {
        if (gltfModel.Name.empty())
        {
            FOO_ENGINE_WARN("Model name empty");
            return;
        }
        auto& asset = s_ModelMap[gltfModel.Name];
        if (asset.Status == AssetStatus::WORKING)
        {
            return;
        }
        asset.Status = AssetStatus::WORKING;

        for (auto& mat : gltfModel.Materials)
        {
            AssetManager::AddMaterial(mat);
        }

        std::string UnnamedImageStr("Unnamed Image");
        for (auto& gltfImage : gltfModel.ImageSources)
        {
            LoadTexture(gltfImage.Name, (void*)gltfImage.ImageBuffer, gltfImage.ImageSize,
                        gltfImage.Width, gltfImage.Height);
        }
        for (auto& m : gltfModel.Materials)
        {
            InsertMaterial(m);
        }

        auto model  = std::make_shared<Model>(std::move(gltfModel.Meshes));
        model->Name = gltfModel.Name;
        AssetContainer<std::shared_ptr<Model>> c;
        c.Asset  = Shared<Model>(model);
        c.Status = AssetStatus::READY;

        s_ModelMap[gltfModel.Name] = c;
        Renderer3D::SubmitModel(model);
    }
    void AssetManager::LoadObjModel(Unique<ObjModel> objModel)
    {
        if (HasModelExists(objModel->Name))
        {
            return;
        }

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
                auto& texPathStr = mat.BaseColorTexture.Name;
                if (texPathStr.empty())
                {
                    continue;
                }
                // auto texPath = mat.PbrMat.BaseColorTexturePath;

                // LoadTexture(texPath, mat.PbrMat.BaseColorTextureName);
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
    void AssetManager::WriteBmpFile(const std::filesystem::path& path, void* data, int w, int h)
    {
        stbi_write_bmp(path.string().c_str(), w, h, STBI_rgb_alpha, data);
    }
    Asset::FImage AssetManager::CreateFimageAssetFile(const String& assetName, size_t imageSize,
                                                      i32 w, i32 h, void* pixelData)
    {
        Asset::FImage fimage;
        fimage.Name         = assetName;
        fimage.Size         = imageSize;
        fimage.Width        = w;
        fimage.Height       = h;
        fimage.ChannelCount = STBI_rgb_alpha;
        fimage.Format       = Asset::TextureFormat::RGBA8;

        auto images    = File::GetImagesPath();
        auto assetFile = images / fimage.Name;
        assetFile.replace_extension(".fimgbuff");
        fimage.BufferPath = assetFile.string();
        if (!std::filesystem::exists(assetFile))
        {
            WriteBmpFile(assetFile, pixelData, fimage.Width, fimage.Height);

            ImageSerializer is;
            auto j = is.Serialize(fimage);

            assetFile.replace_extension(".fimg");
            std::ofstream o{assetFile};
            DEFER(o.close());

            o << std::setw(4) << j << std::endl;
        }
        return fimage;
    }

    void AssetManager::LoadTexture(const std::filesystem::path& path)
    {
        if (HasTextureExists(path.string()))
        {
            FOO_ENGINE_WARN("Texture {0} is already loaded", path.filename().string());
            return;
        }
        auto* pRenderDevice = Backend::GetRenderDevice();

        auto pathStr = path.string();
        i32 texWidth, texHeight, texChannels;
        stbi_uc* pixels =
            stbi_load(pathStr.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        size_t imageSize = texWidth * texHeight * 4;

        Defer d{[&] { stbi_image_free(pixels); }};
        if (!pixels)
        {
            FOO_ENGINE_ERROR("Coult not load image : {0}", path.string());
            return;
        }

        LoadTexture(path.filename().string(), pixels, imageSize, texWidth, texHeight);
    }

    size_t g_UnnamedImagesCount = 0;
    void AssetManager::LoadTexture(Asset::FImage& fimage, void* pixels)
    {
        if (HasTextureExists(fimage.Name))
        {
            g_UnnamedImagesCount++;
            fimage.Name += std::to_string(g_UnnamedImagesCount);
        }

        auto* pRenderDevice        = Backend::GetRenderDevice();
        const auto* physicalDevice = pRenderDevice->GetPhysicalDevice();
        VulkanBuffer::BuffDesc stageDesc{};
        stageDesc.pRenderDevice   = pRenderDevice;
        stageDesc.Usage           = Vulkan::BUFFER_USAGE_TRANSFER_SOURCE;
        stageDesc.MemoryFlag      = Vulkan::BUFFER_MEMORY_FLAG_CPU_VISIBLE;
        stageDesc.Name            = std::string("SB Texture: ") + fimage.Name;
        stageDesc.BufferData.Data = nullptr;
        stageDesc.BufferData.Size = fimage.Size;

        VulkanBuffer stageBuffer{stageDesc};
        stageBuffer.MapMemory();
        stageBuffer.UpdateData(pixels, fimage.Size);
        stageBuffer.UnMapMemory();

        VulkanTexture::CreateInfo ci{};
        ci.pRenderDevice = pRenderDevice;
        ci.MaxAnisotropy = physicalDevice->GetDeviceProperties().limits.maxSamplerAnisotropy;
        ci.Name          = fimage.Name;
        ci.AspectFlags   = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.Format        = VK_FORMAT_R8G8B8A8_SRGB;
        ci.MemoryPropertiesFlags = Vulkan::BUFFER_MEMORY_FLAG_GPU_ONLY;
        ci.Tiling                = VK_IMAGE_TILING_LINEAR;
        ci.UsageFlags            = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        ci.Extent = {static_cast<uint32_t>(fimage.Width), static_cast<uint32_t>(fimage.Height)};
        ci.Size   = fimage.Size;
        ci.Width  = fimage.Height;
        ci.Height = fimage.Height;
        ci.ChannelCount = STBI_rgb_alpha;

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
        InsertTextureToVector(texPtr, fimage.Name);
    }
    void AssetManager::LoadTexture(const String& name, void* buffer, size_t size, i32 w, i32 h)
    {
        auto fimg = CreateFimageAssetFile(name, size, w, h, buffer);
        fimg.Name = std::filesystem::path(fimg.Name).filename().replace_extension("").string();
        LoadTexture(fimg, buffer);
    }

    void AssetManager::InsertTextureToVector(const std::shared_ptr<VulkanTexture>& pT,
                                             const std::string& name)
    {
        std::lock_guard<std::mutex> lock(g_Texture_mutex);
        s_TextureMap[name] = pT;
    }

    Asset::FMaterial* AssetManager::GetMaterial(const std::string& name)
    {
        if (HasMaterialExists(name))
        {
            return &s_MaterialMap[name];
        }
        return nullptr;
    }

    void AssetManager::InsertMaterial(const Asset::FMaterial& m)
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
    Asset::FMaterial* AssetManager::GetDefaultMaterial()
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
        float data[] = {199.0f, 199.f, 199.f, 255.f};
        Asset::FImage defaultTex;
        defaultTex.Name         = DEFAULT_TEXTURE_NAME;
        defaultTex.ChannelCount = STBI_rgb_alpha;
        defaultTex.Width        = 1;
        defaultTex.Height       = 1;
        defaultTex.Size         = sizeof(data);
        LoadTexture(defaultTex, data);
        Asset::FMaterial m;
        m.Name                  = DEFAULT_MATERIAL_NAME;
        m.BaseColorTexture.Name = DEFAULT_TEXTURE_NAME;
        InsertMaterial(m);
        g_IsDefaulTextureInitialized = true;
    }
    bool AssetManager::HasTextureExists(const std::string& name)
    {
        if (s_TextureMap.find(name) != s_TextureMap.end())
        {
            return true;
        }
        return false;
    }
    bool AssetManager::HasMaterialExists(const std::string& name)
    {
        if (s_MaterialMap.find(name) != s_MaterialMap.end())
        {
            return true;
        }
        return false;
    }
    bool AssetManager::HasModelExists(const std::string& name)
    {
        if (s_ModelMap.find(name) != s_ModelMap.end())
        {
            return true;
        }
        return false;
    }

}  // namespace FooGame
