#include "AssetManager.h"
#include <Log.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <tiny_gltf.h>
#include <tiny_obj_loader.h>
#include <pch.h>
#include <filesystem>
#include "ObjLoader.h"
#include "Thread.h"
#include "../Engine/Geometry/Model.h"
#include "../Engine/Core/VulkanTexture.h"
#include "../Engine/Engine/Backend.h"
#include "../Base.h"
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
#include "../Core/Assert.h"
#include "../Core/UUID.h"
#include "../Config.h"
#include "src/Core/Application.h"
namespace FooGame
{
    std::mutex g_Texture_mutex;
    std::mutex g_Model_mutex;
    std::mutex g_Material_mutex;

    ModelRegistery s_ModelMap;
    TextureRegistery s_TextureMap;
    MaterialRegistery s_MaterialMap;

    Unique<Thread> s_pThread;

    void AssetManager::Init()
    {
        s_pThread = Thread::Create();
        CreateDefaultTexture();
    }
    bool IsFileExists(const std::filesystem::path& p)
    {
        return std::filesystem::exists(p);
    }
    void AssetManager::DeInit()
    {
        s_pThread.reset();
    }

    MaterialRegistery& AssetManager::GetAllMaterials()
    {
        return s_MaterialMap;
    }

    const TextureRegistery& AssetManager::GetAllImages()
    {
        return s_TextureMap;
    }

    void AssetManager::LoadModel(const Asset::FModel& fmodel, UUID id)
    {
        if (HasModelAssetExists(id))
        {
            return;
        }

        List<Mesh> meshes;
        for (auto& m : fmodel.Meshes)
        {
            Mesh mesh;
            mesh.MaterialId = m.MaterialId;
            mesh.Name       = m.Name;
            mesh.Vertices.reserve(m.VertexCount);
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
                mesh.Vertices.emplace_back(std::move(v));
            }
            mesh.Indices = std::move(m.Indices);
            meshes.emplace_back(std::move(mesh));
        }

        auto model  = std::make_shared<Model>(std::move(meshes));
        model->Name = fmodel.Name;
        InsertModelAsset(model, id);

        Backend::SubmitToRenderThread([=]() { Renderer3D::SubmitModel(id); });
    }

    void AssetManager::LoadGLTFModel(GltfModel& gltfModel, UUID id)
    {
        if (gltfModel.Name.empty())
        {
            FOO_ENGINE_WARN("Model name empty");
            return;
        }

        Hashmap<String, u64> imagePairs;

        String UnnamedImageStr("Unnamed Image");
        for (auto& gltfImage : gltfModel.ImageSources)
        {
            auto imageName = gltfImage.Name;
            auto path      = File::GetImagesPath() / imageName;
            path.replace_extension(FIMAGE_BUFFER_EXTENSION);
            Asset::FImage fimage;
            auto id                 = UUID();
            fimage.Name             = gltfImage.Name;
            fimage.Size             = gltfImage.ImageSize;
            fimage.Height           = gltfImage.Height;
            fimage.Width            = gltfImage.Width;
            imagePairs[fimage.Name] = id;
            CreateFIMGFile(path, fimage, gltfImage.ImageBuffer);

            LoadFIMG(fimage, id);
        }
        List<UUID> ids;

        for (auto& mat : gltfModel.Materials)
        {
            auto* fmat               = new Asset::FMaterial();
            fmat->RoughnessTextureId = imagePairs[mat.RoughnessTextureName];

            fmat->BaseColorTexture.id = imagePairs[mat.BaseColorTextureName];

            fmat->MetallicTextureId = imagePairs[mat.MetallicTextureName];

            fmat->BaseColorTexture.Name      = mat.BaseColorTextureName;
            fmat->BaseColorTexture.factor[0] = mat.BaseColorTextureFactor[0];
            fmat->BaseColorTexture.factor[1] = mat.BaseColorTextureFactor[1];
            fmat->BaseColorTexture.factor[2] = mat.BaseColorTextureFactor[2];
            fmat->BaseColorTexture.factor[3] = mat.BaseColorTextureFactor[3];

            fmat->NormalTextureId = DEFAULT_TEXTURE_ID;

            fmat->RoughnessFactor = mat.RoughnessFactor;
            fmat->MetallicFactor  = mat.MetallicFactor;

            fmat->Name = mat.Name;

            auto id = UUID();
            ids.push_back(id);
            AddMaterial(Shared<Asset::FMaterial>(fmat), id);
        }
        if (!ids.empty())
        {
            auto idSize = ids.size();
            for (size_t i = 0; i < gltfModel.Meshes.size(); i++)
            {
                gltfModel.Meshes[i].MaterialId = ids[i % idSize];
            }
        }

        auto* model = new Model(std::move(gltfModel.Meshes));
        model->Name = gltfModel.Name;
        InsertModelAsset(Shared<Model>(model), id);

        Renderer3D::SubmitModel(id);
    }
    void AssetManager::LoadObjModel(Unique<ObjModel> objModel, UUID id)
    {
        for (auto& mesh : objModel->Meshes)
        {
            mesh.MaterialId = DEFAULT_MATERIAL_ID;
        }

        auto modelPtr = std::make_shared<Model>();

        for (size_t i = 0; i < objModel->Meshes.size(); i++)
        {
            modelPtr->Meshes.emplace_back(std::move(objModel->Meshes[i]));
        }
        modelPtr->Name = objModel->Name;

        Application::Get().SubmitToMainThread(
            [=]()
            {
                InsertModelAsset(modelPtr, id);
                Renderer3D::SubmitModel(id);
            });
    }
    void AssetManager::LoadFIMG(const Asset::FImage& fimage, UUID id)
    {
        if (HasTextureExists(id))
        {
            return;
        }
        auto fimageDataPath =
            File::GetImagesPath() /
            std::filesystem::path(fimage.Name).replace_extension(FIMAGE_BUFFER_EXTENSION);
        auto fimageDataPathStr = fimageDataPath.string();

        i32 texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(fimageDataPathStr.c_str(), &texWidth, &texHeight, &texChannels,
                                    STBI_rgb_alpha);
        DEFER(stbi_image_free(pixels));
        size_t imageSize = texWidth * texHeight * 4;
        FOO_ASSERT(texWidth == fimage.Width);
        FOO_ASSERT(texHeight == fimage.Height);
        FOO_ASSERT(imageSize == fimage.Size);

        LoadTexture(fimage.Name, pixels, imageSize, texWidth, texHeight, id);
    }
    void AssetManager::CreateFIMGFile(std::filesystem::path& assetPath, const Asset::FImage& fimage,
                                      void* pixels)
    {
        assetPath.replace_extension(FIMAGE_BUFFER_EXTENSION);
        if (!IsFileExists(assetPath))
        {
            WriteBmpFile(assetPath, pixels, fimage.Width, fimage.Height);
        }
        ImageSerializer is;
        auto j = is.Serialize(fimage);

        assetPath.replace_extension(FIMAGE_ASSET_EXTENSION);
        std::ofstream o{assetPath};
        DEFER(o.close());

        o << std::setw(4) << j << std::endl;
    }
    void AssetManager::LoadExternalImage(const std::filesystem::path& path)
    {
        auto imageName = path.filename().string();
        auto pathStr   = path.string();

        i32 texWidth, texHeight, texChannels;
        stbi_uc* pixels =
            stbi_load(pathStr.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        size_t imageSize = texWidth * texHeight * 4;

        Defer d{[&] { stbi_image_free(pixels); }};
        auto id = UUID();
        Asset::FImage fimage =
            CreateFimageAssetFile(imageName, imageSize, texWidth, texHeight, pixels, id);
        LoadFIMG(fimage, id);
    }
    void AssetManager::WriteBmpFile(const std::filesystem::path& path, void* data, int w, int h)
    {
        stbi_write_bmp(path.string().c_str(), w, h, STBI_rgb_alpha, data);
    }
    Asset::FImage AssetManager::CreateFimageAssetFile(const String& assetName, size_t imageSize,
                                                      i32 w, i32 h, void* pixelData, UUID id)
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
        CreateFIMGFile(assetFile, fimage, pixelData);
        return fimage;
    }
    void AssetManager::LoadTexture(const String& name, unsigned char* buffer, size_t size,
                                   i32 width, i32 height, UUID id)
    {
        auto* pRenderDevice        = Backend::GetRenderDevice();
        const auto* physicalDevice = pRenderDevice->GetPhysicalDevice();
        VulkanBuffer::BuffDesc stageDesc{};
        stageDesc.pRenderDevice   = pRenderDevice;
        stageDesc.Usage           = Vulkan::BUFFER_USAGE_TRANSFER_SOURCE;
        stageDesc.MemoryFlag      = Vulkan::BUFFER_MEMORY_FLAG_CPU_VISIBLE;
        stageDesc.Name            = std::string("SB Texture: ") + name;
        stageDesc.BufferData.Data = nullptr;
        stageDesc.BufferData.Size = size;

        VulkanBuffer stageBuffer{stageDesc};
        stageBuffer.MapMemory();
        stageBuffer.UpdateData(buffer, size);
        stageBuffer.UnMapMemory();

        VulkanTexture::CreateInfo ci{};
        ci.pRenderDevice = pRenderDevice;
        ci.MaxAnisotropy = physicalDevice->GetDeviceProperties().limits.maxSamplerAnisotropy;
        ci.Name          = name;
        ci.AspectFlags   = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.Format        = VK_FORMAT_R8G8B8A8_SRGB;
        ci.MemoryPropertiesFlags = Vulkan::BUFFER_MEMORY_FLAG_GPU_ONLY;
        ci.Tiling                = VK_IMAGE_TILING_LINEAR;
        ci.UsageFlags            = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        ci.Extent       = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        ci.Size         = size;
        ci.Width        = width;
        ci.Height       = height;
        ci.ChannelCount = STBI_rgb_alpha;

        auto* vTexture = new VulkanTexture{ci};
        auto vkImage   = vTexture->GetImage();
        {
            std::lock_guard<std::mutex> lock(g_Texture_mutex);

            Backend::TransitionImageLayout(vkImage->GetImageHandle(), VK_FORMAT_R8G8B8A8_SRGB,
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            Backend::CopyBufferToImage(stageBuffer, *vTexture);
            Backend::TransitionImageLayout(vkImage->GetImageHandle(), VK_FORMAT_R8G8B8A8_SRGB,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        InsertTextureAsset(Shared<VulkanTexture>(vTexture), id);
    }

    size_t g_UnnamedImagesCount = 0;

    void AssetManager::InsertTextureAsset(const Shared<VulkanTexture>& pT, UUID id)
    {
        std::lock_guard<std::mutex> lock(g_Texture_mutex);
        if (!HasTextureExists(id))
        {
            s_TextureMap.insert(TextureRegistery::value_type{
                id, {Asset::AssetStatus::READY, pT, pT->GetName(), id}
            });
        }
    }

    AssetMaterialC* AssetManager::GetMaterialAsset(const u64& id)
    {
        if (HasMaterialExists(id))
        {
            return &s_MaterialMap[id];
        }
        return nullptr;
    }

    void AssetManager::AddMaterial(Shared<Asset::FMaterial> material, UUID id)
    {
        std::lock_guard<std::mutex> lock(g_Material_mutex);
        if (!HasMaterialExists(id))
        {
            s_MaterialMap.insert(MaterialRegistery::value_type{
                id, {Asset::AssetStatus::READY, material, material->Name, id}
            });
        }
    }
    void AssetManager::LoadGLTFModelAsync(std::string path, std::string name, bool isGlb)
    {
    }

    void AssetManager::InsertModelAsset(Shared<Model> m, UUID id)
    {
        std::lock_guard<std::mutex> lock(g_Model_mutex);
        s_ModelMap[id] = {Asset::AssetStatus::READY, m, m->Name, id};
    }
    bool AssetManager::HasModelAssetExists(UUID id)
    {
        if (s_ModelMap.find(id) != s_ModelMap.end())
        {
            return true;
        }
        return false;
    }

    AssetModelC* AssetManager::GetModelAsset(UUID id)
    {
        if (HasModelAssetExists(id))
        {
            return &s_ModelMap[id];
        }
        FOO_ASSERT("Asset could not found");
        return nullptr;
    }

    AssetTextureC* AssetManager::GetTextureAsset(u64 id)
    {
        if (HasTextureExists(id))
        {
            return &s_TextureMap[id];
        }
        return nullptr;
    }
    bool g_IsDefaulTextureInitialized = false;
    AssetMaterialC* AssetManager::GetDefaultMaterial()
    {
        return &s_MaterialMap[DEFAULT_MATERIAL_ID];
    }

    AssetTextureC* AssetManager::GetDefaultTextureAsset()
    {
        FOO_ASSERT(g_IsDefaulTextureInitialized, "Default Asset did not created broo");
        return GetTextureAsset(DEFAULT_TEXTURE_ID);
    }

    void AssetManager::CreateDefaultTexture()
    {
        float data[] = {199.0f, 199.f, 199.f, 255.f};
        LoadTexture(DEFAULT_TEXTURE_NAME, (unsigned char*)data, sizeof(data), 1, 1,
                    DEFAULT_TEXTURE_ID);
        Asset::FMaterial m;
        m.Name               = DEFAULT_MATERIAL_NAME;
        m.NormalTextureId    = DEFAULT_TEXTURE_ID;
        m.MetallicTextureId  = DEFAULT_TEXTURE_ID;
        m.RoughnessTextureId = DEFAULT_TEXTURE_ID;

        m.BaseColorTexture.id = DEFAULT_TEXTURE_ID;
        AddMaterial(Shared<Asset::FMaterial>(new Asset::FMaterial(m)), DEFAULT_MATERIAL_ID);
        g_IsDefaulTextureInitialized = true;
    }
    bool AssetManager::HasTextureExists(UUID id)
    {
        if (s_TextureMap.find(id) != s_TextureMap.end())
        {
            return true;
        }
        return false;
    }
    bool AssetManager::HasMaterialExists(UUID id)
    {
        if (s_MaterialMap.find(id) != s_MaterialMap.end())
        {
            return true;
        }
        return false;
    }

}  // namespace FooGame
