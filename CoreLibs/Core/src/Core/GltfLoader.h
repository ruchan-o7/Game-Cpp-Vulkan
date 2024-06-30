#pragma once
#include "../Base.h"
#include "../Engine/Geometry/Mesh.h"
#include "pch.h"
#include <glm/glm.hpp>
#include "../Core/UUID.h"
#include "../Config.h"
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
            UUID imageId = DEFAULT_TEXTURE_ID;
    };

    struct GltfMaterialSource
    {
            String Name;
            glm::vec4 BaseColorFactor = glm::vec4(1.0f);
            u32 BaseColorTexIndex;
            UUID Id = DEFAULT_MATERIAL_ID;
            // String BaseColorTextureName;
            // float BaseColorTextureFactor[4];
            //
            // String NormalTextureName;
            //
            // String RoughnessTextureName;
            // float RoughnessFactor;
            //
            // String MetallicTextureName;
            // float MetallicFactor;
    };
    struct GltfPrimitive
    {
            u32 firstIndex;
            u32 indexCount;
            i32 materialIndex;
    };
    struct GltfMesh
    {
            String Name;
            glm::mat4 TransformMatrix;
            List<GltfPrimitive> primitives;
    };
    struct Node
    {
            Node* Parent;
            List<Node*> Children;
            GltfMesh Mesh;
            glm::mat4 Matrix;
            ~Node()
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
            List<i32> Textures;
            List<Node*> Nodes;
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
