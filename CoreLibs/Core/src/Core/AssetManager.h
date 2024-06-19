#pragma once
#include <string>
#include "../Engine/Geometry/Material.h"

namespace FooGame
{
    class Model;
    class VulkanTexture;

    class AssetManager
    {
        public:
            static void LoadGLTFModel(const std::string& path, const std::string& name, bool isGlb);
            static void LoadObjModel(const std::string& path, const std::string& modelName);
            static void LoadTexture(const std::string& name, void* pixels, size_t size,
                                    int32_t width, int32_t height);
            static void LoadTexture(const std::string& path, const std::string& name);
            static void CreateDefaultTexture();

        public:
            static std::shared_ptr<Model> GetModel(const std::string& name);
            static Material GetMaterial(const std::string& name);
            static std::shared_ptr<VulkanTexture> GetTexture(const std::string& name);
            static std::shared_ptr<VulkanTexture> GetDefaultTexture();

        private:
            static void InsertTextureToVector(const std::shared_ptr<VulkanTexture>& pT,
                                              const std::string& name);
            static void InsertMaterial(const std::string& name, const Material& m);
            static void InsertMap(const std::string& name, std::shared_ptr<Model> m);
    };
}  // namespace FooGame
