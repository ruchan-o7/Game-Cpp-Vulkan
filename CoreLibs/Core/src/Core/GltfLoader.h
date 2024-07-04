#pragma once
#include "../Base.h"
#include "pch.h"
#include <glm/glm.hpp>
#include "../Engine/Geometry/Vertex.h"
namespace FooGame
{
    struct Material;
    struct GltfImageSource
    {
            unsigned char* ImageBuffer;
            size_t ImageSize;
            String Name;
            u32 Width, Height;
            i32 ComponentCount;
    };
    struct GltfTexture
    {
            i32 imageIndex;
    };

    struct GltfMaterialSource
    {
            String Name;
            glm::vec4 BaseColorFactor = glm::vec4(1.0f);
            u32 BaseColorTextureIndex;
    };
    struct GltfPrimitive
    {
            u32 firstIndex;
            u32 indexCount;
            i32 materialIndex;
            // glm::mat4 TransformMatrix;
    };
    struct GltfMesh
    {
            String Name;
            List<GltfPrimitive> primitives;
    };
    struct GltfNode
    {
            GltfNode* Parent;
            List<GltfNode*> Children;
            GltfMesh Mesh;
            glm::mat4 Matrix;
            ~GltfNode()
            {
                for (auto& c : Children)
                {
                    delete c;
                }
            }
    };
    struct GltfModel
    {
            String Name;
            List<GltfMesh> Meshes;
            List<GltfImageSource> ImageSources;
            List<GltfMaterialSource> Materials;
            List<GltfTexture> Textures;
            List<GltfNode*> Nodes;
            List<Vertex> Vertices;
            List<u32> Indices;
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
