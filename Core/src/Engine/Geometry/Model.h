#pragma once
#include "Vertex.h"
#include "../../Base.h"
#include "../../Scene/Asset.h"
namespace FooGame
{
    struct Model
    {
            DELETE_COPY(Model);
            Model& operator=(Model&& o)
            {
                if (this != &o)
                {
                    this->Meshes = std::move(o.Meshes);
                    this->Name   = std::move(o.Name);
                    // TODO:
                }
                return *this;
            }
            Model() = default;

            List<Asset::FMesh> Meshes;
            List<Vertex> Vertices;
            List<u32> Indices;
            String Name  = "Unspecified Model Name";
            u32 RenderId = 0;
    };

}  // namespace FooGame
