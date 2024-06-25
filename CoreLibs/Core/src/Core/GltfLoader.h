#pragma once
#include "../Base.h"
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

    struct GltfMaterialSource
    {
            String Name;
            String BaseColorTextureName;
            float BaseColorTextureFactor[4];

            String NormalTextureName;

            String RoughnessTextureName;
            float RoughnessFactor;

            String MetallicTextureName;
            float MetallicFactor;
    };
    struct GltfModel
    {
            std::string Name;
            std::vector<GltfImageSource> ImageSources;
            std::vector<Mesh> Meshes;
            std::vector<GltfMaterialSource> Materials;
            ~GltfModel()
            {
                for (auto& is : ImageSources)
                {
                    delete is.ImageBuffer;
                }
                ImageSources.clear();
                Meshes.clear();
                Materials.clear();
            }
    };
    class GltfLoader
    {
        public:
            GltfLoader(const std::filesystem::path& path, bool isGlb);
            Unique<GltfModel> Load() const;

        private:
            std::filesystem::path m_Path;
            std::string m_Name;
            bool m_IsGlb;
            bool m_EligibleToLoad;
    };

}  // namespace FooGame
