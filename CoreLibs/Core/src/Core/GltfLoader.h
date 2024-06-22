#pragma once
#include "pch.h"
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
            std::vector<Material> Materials;
            std::string Name;
    };
    class GltfLoader
    {
        public:
            GltfLoader(const std::string& path, const std::string& name, bool isGlb);
            GltfModel* Load() const;

        private:
            std::string m_Path, m_Name;
            bool m_IsGlb;
            bool m_EligibleToLoad;
    };

}  // namespace FooGame
