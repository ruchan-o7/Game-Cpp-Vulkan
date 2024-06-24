#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include "../Engine/Geometry/Material.h"
#include "../Scene/Asset.h"
#include "Base.h"
namespace FooGame
{
    class Model;
    class Mesh;
    class VulkanTexture;
    class GltfLoader;
    class ObjLoader;
    class VulkanImage;
    class GltfModel;
    struct ObjModel;
    class AssetManager
    {
        public:
            static void Init();
            static void DeInit();

        public:
            static void LoadModel(const Asset::FModel& fmodel);
            static void LoadGLTFModel(GltfModel& gltfModel);
            static void LoadGLTFModelAsync(std::string path, std::string name, bool isGlb);

            static void LoadObjModel(Unique<ObjModel> objModel);

            static void LoadTexture(Asset::FImage& fimage, void* pixels);

            static void LoadTexture(const String& name, void* buffer, size_t size, i32 w, i32 h);

            static void LoadTexture(const std::filesystem::path& path);

            static void CreateDefaultTexture();
            static void AddMaterial(Material material);

        public:
            static std::shared_ptr<Model> GetModel(const std::string& name);

            static AssetContainer<std::shared_ptr<Model>>& GetModelAsset(const std::string& name);

            static AssetContainer<std::shared_ptr<Mesh>>& GetMeshAsset(const std::string& name);

            static Material* GetMaterial(const std::string& name);

            static std::shared_ptr<VulkanTexture> GetTexture(const std::string& name);

            static std::shared_ptr<VulkanTexture> GetDefaultTexture();
            static Material* GetDefaultMaterial();

            static std::unordered_map<std::string, Material>& GetAllMaterials();
            static const std::unordered_map<std::string, std::shared_ptr<VulkanTexture>>&
            GetAllImages();

            static bool HasTextureExists(const std::string& name);
            static bool HasMaterialExists(const std::string& name);

        private:
            static void InsertTextureToVector(const std::shared_ptr<VulkanTexture>& pT,
                                              const std::string& name);
            static void InsertMaterial(const Material& m);
            static void InsertModel(const std::string& name, std::shared_ptr<Model> m);
            friend class VulkanTexture;
            // clang-format off
            static Asset::FImage CreateFimageAssetFile(
                const String& assetName,
                size_t imageSize,
                i32 w,i32 h, void* pixelData
            );
            // clang-format on
            static void WriteBmpFile(const std::filesystem::path& path, void* data, int w, int h);
    };
}  // namespace FooGame
