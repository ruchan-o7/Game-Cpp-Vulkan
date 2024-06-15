#pragma once
#include <string>
#include <unordered_map>
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
            static void AddMaterial(const Material2& material);

            static std::unordered_map<std::string, Material2> AllMaterials();

        public:
            static std::shared_ptr<Model> GetModel(const std::string& name);
            static Material2 GetMaterial(const std::string& name);
            static std::shared_ptr<VulkanTexture> GetTexture(const std::string& name);
    };
}  // namespace FooGame
