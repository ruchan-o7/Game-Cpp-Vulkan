#pragma once
#include <filesystem>
#include "../Scene/Asset.h"
#include "../Base.h"
#include "../Engine/Geometry/Model.h"
#include "../Engine/Geometry/Vertex.h"
namespace FooGame
{
    class Mesh;
    class VulkanTexture;
    class GltfLoader;
    class ObjLoader;
    class VulkanImage;
    class GltfModel;
    struct ObjModel;

    using AssetModelC    = Asset::AssetContainer<Model>;
    using AssetTextureC  = Asset::AssetContainer<VulkanTexture>;
    using AssetMaterialC = Asset::AssetContainer<Asset::FMaterial>;

    using ModelRegistery    = Hashmap<UUID, AssetModelC>;
    using TextureRegistery  = Hashmap<UUID, AssetTextureC>;
    using MaterialRegistery = Hashmap<UUID, AssetMaterialC>;

    class AssetManager
    {
        public:
            static void Init();
            static void DeInit();

        public:
            static void LoadModel(Model& fmodel, UUID id, List<Vertex>&& vertices,
                                  List<u32>&& indices);
            static void LoadGLTFModel(GltfModel& gltfModel, UUID id);
            static void LoadGLTFModelAsync(GltfLoader loader, UUID id);

            static void LoadObjModel(Unique<ObjModel> objModel, UUID id);

            static void LoadFIMG(const Asset::FImage& fimage, UUID id);
            static void LoadExternalImage(const std::filesystem::path& path);
            static void LoadTexture(const String& name, unsigned char* buffer, size_t size,
                                    i32 width, i32 height, UUID id);

            static void CreateDefaultTexture();

            static void AddMaterial(Asset::FMaterial&& material);

        public:
            static AssetModelC* GetModelAsset(UUID id);

            static AssetMaterialC* GetMaterialAsset(const u64& id);

            static AssetTextureC* GetTextureAsset(u64 id);

            static AssetTextureC* GetDefaultTextureAsset();

            static AssetMaterialC* GetDefaultMaterial();

            static MaterialRegistery& GetAllMaterials();

            static const TextureRegistery& GetAllImages();
            static ModelRegistery& GetAllModels();

            static bool HasTextureExists(UUID id);
            static bool HasMaterialExists(UUID id);
            static bool HasModelAssetExists(UUID id);

        private:
            static void InsertTextureAsset(Shared<VulkanTexture> t, UUID id);
            static void InsertMaterialAsset(const Asset::FMaterial& m);
            static void InsertModelAsset(Model&& m, UUID id);
            // clang-format off
            static Asset::FImage CreateFimageAssetFile(
                const String& assetName,
                size_t imageSize,
                i32 w,i32 h, void* pixelData,UUID id
            );
            // clang-format on
            static void WriteBmpFile(const std::filesystem::path& path, void* data, int w, int h);
            static void CreateFIMGFile(std::filesystem::path& assetPath,
                                       const Asset::FImage& fimage, void* pixels);

            friend class VulkanTexture;
    };
}  // namespace FooGame
