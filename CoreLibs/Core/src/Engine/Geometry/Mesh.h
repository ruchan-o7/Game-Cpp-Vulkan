#pragma once
#include <pch.h>
#include "Vertex.h"
#include "../../Base.h"
namespace FooGame
{

    struct Mesh
    {
        public:
            DELETE_COPY(Mesh);
            Mesh();
            ~Mesh();

            Mesh(Mesh&& o)
            {
                Vertices     = std::move(o.Vertices);
                Indices      = std::move(o.Indices);
                MaterialName = std::move(o.MaterialName);
                Name         = std::move(o.Name);
                RenderId     = o.RenderId;
                o.RenderId   = -1;
            }
            Mesh& operator=(Mesh&& other) noexcept
            {
                if (this != &other)
                {
                    this->MaterialName = std::move(other.MaterialName);
                    this->Name         = std::move(other.Name);
                    this->Indices      = std::move(other.Indices);
                    this->Vertices     = std::move(other.Vertices);
                    this->RenderId     = other.RenderId;

                    other.RenderId = 0;
                }
                return *this;
            }

            Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);

            Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
            std::vector<Vertex> Vertices;
            std::vector<uint32_t> Indices;
            std::string MaterialName;
            u64 MaterialId;
            std::string Name;
            uint32_t RenderId;
    };

}  // namespace FooGame
