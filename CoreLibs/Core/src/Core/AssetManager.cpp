#include "AssetManager.h"
#include <Log.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <tiny_gltf.h>
#include <tiny_obj_loader.h>
#include <pch.h>
#include <cstddef>
#include <filesystem>
#include <utility>
#include "ObjLoader.h"
#include "Thread.h"
#include "../Engine/Geometry/Model.h"
#include "../Engine/Core/VulkanTexture.h"
#include "../Engine/Engine/Backend.h"
#include "../Base.h"
#include "../Engine/Core/Types.h"
#include "../Engine/Core/VulkanBuffer.h"
#include "../Engine/Engine/Renderer3D.h"
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
#include "../Core/Application.h"
#include "../Engine/Core/VulkanTexture.h"
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
    ModelRegistery& AssetManager::GetAllModels()
    {
        return s_ModelMap;
    }

    void AssetManager::LoadModel(Model& fmodel, UUID id, List<Vertex>&& vertices,
                                 List<u32>&& indices)
    {
        if (HasModelAssetExists(id))
        {
            return;
        }
        InsertModelAsset(std::move(fmodel), id);

        Backend::SubmitToRenderThread(  // TODO: copying maybe much better
            [&]() { Renderer3D::SubmitModel(id, std::move(vertices), std::move(indices)); });
    }
    void AssetManager::LoadGLTFModelAsync(GltfLoader loader, UUID id)
    {
        s_pThread->AddTask(
            [=]()
            {
                Shared<GltfModel> gltfModel = loader.Load();

                AssetManager::LoadGLTFModel(*gltfModel, id);
            });
    }
    struct IndexAndId
    {
            i32 index = 0;
            UUID id   = DEFAULT_TEXTURE_ID;
    };
    void ConvertGltfNodeToMesh(GltfNode* input, Model* model, const List<IndexAndId>& matAndId)
    {
        if (input->Children.size() > 0)
        {
            for (const auto& node : input->Children)
            {
                ConvertGltfNodeToMesh(node, model, matAndId);
            }
        }
        Mesh mesh;
        mesh.Name      = input->Mesh.Name;
        mesh.Transform = input->Matrix;
        for (const auto& p : input->Mesh.primitives)
        {
            Mesh::Primitive dp;
            dp.FirstIndex = p.firstIndex;
            dp.IndexCount = p.indexCount;
            dp.MaterialId = matAndId[p.materialIndex].id;
            mesh.Primitives.push_back(dp);
        }
        model->Meshes.push_back(mesh);
    }

    void AssetManager::LoadGLTFModel(GltfModel& gltfModel, UUID id)
    {
        if (gltfModel.Name.empty())
        {
            FOO_ENGINE_WARN("Model name empty");
            return;
        }

        List<IndexAndId> texAndIds;
        texAndIds.reserve(gltfModel.ImageSources.size());
        for (size_t i = 0; i < gltfModel.Textures.size(); i++)
        {
            const auto& gTex = gltfModel.Textures[i];
            const auto& gImg = gltfModel.ImageSources[gTex.imageIndex];

            IndexAndId t;
            t.id    = UUID();
            t.index = gTex.imageIndex;
            texAndIds.push_back(t);

            auto imageName = gImg.Name;
            auto path      = File::GetImagesPath() / imageName;
            path.replace_extension(FIMAGE_BUFFER_EXTENSION);
            Asset::FImage fimage;
            fimage.Name   = gImg.Name;
            fimage.Size   = gImg.ImageSize;
            fimage.Height = gImg.Height;
            fimage.Width  = gImg.Width;

            CreateFIMGFile(path, fimage, gImg.ImageBuffer);

            LoadFIMG(fimage, t.id);
        }

        List<IndexAndId> matAndId;
        for (size_t i = 0; i < gltfModel.Materials.size(); i++)
        {
            const auto& gMat = gltfModel.Materials[i];

            auto tai = texAndIds[gMat.BaseColorTextureIndex];
            Asset::FMaterial fmat{};
            IndexAndId iai;
            iai.index = i;
            iai.id    = UUID();
            matAndId.push_back(iai);

            fmat.RoughnessTextureId = DEFAULT_TEXTURE_ID;

            fmat.BaseColorTexture.id   = tai.id;
            fmat.BaseColorTexture.Name = GetTextureAsset(tai.id)->Asset->GetName();
            fmat.MetallicTextureId     = DEFAULT_TEXTURE_ID;
            fmat.NormalTextureId       = DEFAULT_TEXTURE_ID;

            fmat.BaseColorTexture.factor[0] = gMat.BaseColorFactor[0];
            fmat.BaseColorTexture.factor[1] = gMat.BaseColorFactor[1];
            fmat.BaseColorTexture.factor[2] = gMat.BaseColorFactor[2];
            fmat.BaseColorTexture.factor[3] = gMat.BaseColorFactor[3];

            fmat.Name = gMat.Name;

            AddMaterial(std::move(fmat));
        }
        Model model{};
        model.Name = gltfModel.Name;
        if (gltfModel.Nodes.size() > 0)
        {
            GltfNode* rootNode = gltfModel.Nodes[0];
            ConvertGltfNodeToMesh(rootNode, &model, matAndId);
        }
        InsertModelAsset(std::move(model), id);

        Application::Get().SubmitToMainThread(
            [=] { Renderer3D::SubmitModel(id, gltfModel.Vertices, gltfModel.Indices); });
    }
    void AssetManager::LoadObjModel(Unique<ObjModel> objModel, UUID id)
    {
        Model model{};
        model.Meshes = std::move(objModel->Meshes);
        model.Name   = objModel->Name;
        InsertModelAsset(std::move(model), id);
        auto vertices = objModel->Vertices;
        auto indices  = objModel->Indices;
        Application::Get().SubmitToMainThread([=]()
                                              { Renderer3D::SubmitModel(id, vertices, indices); });
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
#define WRITE_AS_JPG
    void AssetManager::WriteBmpFile(const std::filesystem::path& path, void* data, int w, int h)
    {
#ifdef WRITE_AS_JPG
        stbi_write_jpg(path.string().c_str(), w, h, STBI_rgb_alpha, data, 80);
#else
        stbi_write_bmp(path.string().c_str(), w, h, STBI_rgb_alpha, data);
#endif
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

    void AssetManager::InsertTextureAsset(Shared<VulkanTexture> t, UUID id)
    {
        std::lock_guard<std::mutex> lock(g_Texture_mutex);
        if (!HasTextureExists(id))
        {
            s_TextureMap[id] = AssetTextureC{Asset::AssetStatus::READY, t};
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

    void AssetManager::AddMaterial(Asset::FMaterial&& material)
    {
        std::lock_guard<std::mutex> lock(g_Material_mutex);
        if (!HasMaterialExists(material.Id))
        {
            s_MaterialMap.insert(MaterialRegistery::value_type{
                material.Id, {Asset::AssetStatus::READY, material}
            });
        }
    }

    void AssetManager::InsertModelAsset(Model&& m, UUID id)
    {
        std::lock_guard<std::mutex> lock(g_Model_mutex);
        s_ModelMap[id] = {Asset::AssetStatus::READY, std::move(m)};
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
        return GetDefaultTextureAsset();
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
        m.Id                 = DEFAULT_MATERIAL_ID;
        m.Name               = DEFAULT_MATERIAL_NAME;
        m.NormalTextureId    = DEFAULT_TEXTURE_ID;
        m.MetallicTextureId  = DEFAULT_TEXTURE_ID;
        m.RoughnessTextureId = DEFAULT_TEXTURE_ID;

        m.BaseColorTexture.id = DEFAULT_TEXTURE_ID;
        AddMaterial(std::move(m));
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
