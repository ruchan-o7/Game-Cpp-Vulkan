#pragma once
#include "../Core/Base.h"
#include <Engine.h>
#include <tiny_gltf.h>
namespace FooGame
{
    class AssetLoader
    {
        public:
            static Unique<Mesh> LoadGLTFMesh(const String& path, bool isGlb);
    };

    namespace GLTF
    {
        struct Vertex
        {
                glm::vec3 Position;
                glm::vec3 Normal;
                glm::vec2 Uv;
                glm::vec3 Color;
        };
        struct Primitive
        {
                u32 FirstIndex;
                u32 IndexCount;
                i32 MaterialIndex;
        };
        struct Mesh
        {
                List<Primitive> Primitives;
        };
        struct Material
        {
                glm::vec4 BaseColorFactor = glm::vec4(1.0f);
                u32 BaseColorTextureIndex;
        };
        struct Node
        {
                Node* Parent;
                List<Node*> Children;
                Mesh mesh;
                glm::mat4 Matrix;
                void LoadNode();
                ~Node()
                {
                    for (auto& child : Children)
                    {
                        delete child;
                    }
                }
        };
        class GLTFModel
        {
            public:
                List<Material> Materials;
                List<GLTF::Node*> Nodes;
                void LoadNode(const tinygltf::Node& inputNode,
                              const tinygltf::Model& input, Node* parent,
                              List<u32>& indices, List<GLTF::Vertex>& vertices);
                ~GLTFModel()
                {
                    for (auto node : Nodes)
                    {
                        delete node;
                    }
                }
        };
    }  // namespace GLTF
}  // namespace FooGame
