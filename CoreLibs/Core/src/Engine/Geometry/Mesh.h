#pragma once
#include <cstdint>
#include <vector>
#include "Vertex.h"
#include <vulkan/vulkan.h>
namespace FooGame
{

    struct Primitive
    {
            uint32_t FirstIndex;
            uint32_t IndexCount;
            int32_t MaterialIndex;
    };
    struct Material3;
    struct Mesh
    {
        public:
            Mesh();
            ~Mesh();
            Mesh(Mesh&&);

            Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);

            Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
            std::vector<Vertex> m_Vertices;
            std::vector<uint32_t> m_Indices;
            std::vector<Primitive> MeshPrimitives;
            std::string M3Name;
            uint32_t RenderId;

            uint32_t AlbedoIndex = 0;
    };

}  // namespace FooGame
