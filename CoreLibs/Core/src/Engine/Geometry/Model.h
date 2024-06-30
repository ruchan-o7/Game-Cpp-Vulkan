#pragma once
#include "Mesh.h"
#include "Vertex.h"
namespace FooGame
{
    class VulkanTexture;
    struct Model
    {
            DELETE_COPY(Model);
            Model(std::vector<Mesh>&& meshes);
            Model& operator=(Model&& o)
            {
                if (this != &o)
                {
                    this->Meshes = std::move(o.Meshes);
                    this->Name   = std::move(o.Name);
                }
                return *this;
            }
            Model() = default;

            List<Mesh> Meshes;
            List<Vertex> Vertices;
            List<u32> Indices;
            String Name  = "Unspecified Model Name";
            u32 RenderId = 0;
    };

}  // namespace FooGame
