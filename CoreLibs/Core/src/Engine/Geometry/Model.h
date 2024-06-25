#pragma once
#include "Mesh.h"

namespace FooGame
{
    class VulkanTexture;
    struct Model
    {
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

            std::vector<Mesh> Meshes;
            std::string Name = "Unspecified Model Name";
    };

}  // namespace FooGame
