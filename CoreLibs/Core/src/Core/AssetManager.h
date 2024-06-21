#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include "../Engine/Geometry/Material.h"
#include <unordered_map>
#include "../Scene/Asset.h"
namespace FooGame
{
    class Model;
    class Mesh;
    class VulkanTexture;
    class AssetManager
    {
        public:
            static void Init();
            static void DeInit();

        public:
            static void LoadGLTFModel(std::string path, std::string name, bool isGlb);
            static void LoadGLTFModelAsync(std::string path, std::string name, bool isGlb);
            static void LoadObjModel(const std::filesystem::path& path,
                                     const std::string& modelName, std::string materialName);
            static void LoadTexture(const std::string& name, void* pixels, size_t size,
                                    int32_t width, int32_t height);
            static void LoadTexture(const std::string& path, const std::string& name);
            static void CreateDefaultTexture();
            static void AddMaterial(Material material);

        public:
            static std::shared_ptr<Model> GetModel(const std::string& name);
            static AssetContainer<Model>& GetModelAsset(const std::string& name);
            static AssetContainer<std::shared_ptr<Mesh>>& GetMeshAsset(const std::string& name);
            static Material GetMaterial(const std::string& name);
            static std::shared_ptr<VulkanTexture> GetTexture(const std::string& name);
            static std::shared_ptr<VulkanTexture> GetDefaultTexture();
            static const std::unordered_map<std::string, Material>& GetAllMaterials();

        private:
            static void InsertTextureToVector(const std::shared_ptr<VulkanTexture>& pT,
                                              const std::string& name);
            static void InsertMaterial(const Material& m);
            static void InsertMap(const std::string& name, std::shared_ptr<Model> m);
    };
}  // namespace FooGame
