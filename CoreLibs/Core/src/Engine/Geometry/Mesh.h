#pragma once
#include <cstdint>
#include <utility>
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
            Mesh& operator=(Mesh&& other) noexcept
            {
                if (this != &other)
                {
                    this->AlbedoIndex = other.AlbedoIndex;
                    this->M3Name      = std::move(other.M3Name);
                    this->Name        = std::move(other.Name);
                    this->m_Indices   = std::move(other.m_Indices);
                    this->m_Vertices  = std::move(other.m_Vertices);
                    this->RenderId    = other.RenderId;

                    other.RenderId = 0;
                }
                return *this;
            }

            Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);

            Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
            std::vector<Vertex> m_Vertices;
            std::vector<uint32_t> m_Indices;
            std::vector<Primitive> MeshPrimitives;
            std::string M3Name;
            std::string Name;
            uint32_t RenderId;

            uint32_t AlbedoIndex = 0;
    };

}  // namespace FooGame
