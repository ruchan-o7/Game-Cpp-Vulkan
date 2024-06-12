#pragma once
#include "../Geometry/Model.h"
#include <tiny_gltf.h>
#include <cstdint>
#include <memory>
#include <string>
#include "../Core/VulkanTexture.h"
namespace FooGame
{
    class AssetLoader
    {
        public:
            static std::unique_ptr<Model> LoadGLTFModel(const std::string& path, bool isGlb);
            static std::unique_ptr<Model> LoadObjModel(const std::string& path);
            static std::unique_ptr<VulkanTexture> LoadTexture(void* pixels, size_t size,
                                                              int32_t width, int32_t height);
            static std::unique_ptr<VulkanTexture> LoadTexture(const std::string& path);
    };

}  // namespace FooGame
