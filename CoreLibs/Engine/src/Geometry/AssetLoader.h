#pragma once
#include "../Geometry/Model.h"
#include <tiny_gltf.h>
#include <cstdint>
#include <memory>
#include <string>
namespace FooGame
{
    class AssetLoader
    {
        public:
            // static std::unique_ptr<Mesh> LoadGLTFMesh(const std::string&
            // path,
            //                                           bool isGlb);
            static std::unique_ptr<Model> LoadGLTFModel(const std::string& path,
                                                        bool isGlb);
            static std::unique_ptr<Model> LoadObjModel(const std::string& path);
            static void DestroyTexture(Texture2D& t);
            static Texture2D LoadFromFile(const std::string& path);
            static Texture2D LoadFromBuffer(void* buffer, size_t size,
                                            VkFormat format, int32_t width,
                                            int32_t height);
            static std::unique_ptr<Texture2D> LoadFromFilePtr(
                const std::string& path)
            {
                return std::make_unique<Texture2D>(LoadFromFile(path));
            }
    };

}  // namespace FooGame
