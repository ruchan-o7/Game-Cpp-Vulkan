#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "../Core/VulkanTexture.h"
#include "Mesh.h"

namespace FooGame
{
    class VulkanTexture;
    struct Model
    {
        public:
            Model(std::vector<Mesh>&& meshes);
            Model& operator=(Model&& o)
            {
                if (this != &o)
                {
                    this->AssetId  = o.AssetId;
                    this->m_Meshes = std::move(o.m_Meshes);
                    this->Name     = std::move(o.Name);
                }
                return *this;
            }
            Model() = default;
            std::vector<Mesh>& GetMeshes() { return m_Meshes; }
            std::vector<Mesh> m_Meshes;
            uint32_t AssetId;
            std::string Name = "Model";
    };

}  // namespace FooGame
