#pragma once
#include <filesystem>
#include "pch.h"
#include "../Scene/Asset.h"
namespace FooGame
{
    class Mesh;
    struct Material;
    struct GltfImageSource
    {
            unsigned char* ImageBuffer;
            size_t ImageSize;
            std::string Name;
            uint32_t Width, Height;
            int ComponentCount;
    };
    struct GltfModel
    {
            std::vector<GltfImageSource> ImageSources;
            std::vector<Mesh> Meshes;
            std::vector<Asset::FMaterial> Materials;
            std::string Name;
    };
    class GltfLoader
    {
        public:
            GltfLoader(const std::filesystem::path& path, bool isGlb);
            GltfModel* Load() const;

        private:
            std::filesystem::path m_Path;
            std::string m_Name;
            bool m_IsGlb;
            bool m_EligibleToLoad;
    };

}  // namespace FooGame
