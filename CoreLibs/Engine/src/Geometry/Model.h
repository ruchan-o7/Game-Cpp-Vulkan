#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "Mesh.h"
namespace FooGame
{
    struct Model
    {
        public:
            Model(std::vector<Mesh>&& meshes);
            Model() = default;
            std::vector<Mesh>& GetMeshes() { return m_Meshes; }
            void PushId(uint32_t id) { m_Ids.push_back(id); }
            const std::vector<uint32_t>& GetIds() const { return m_Ids; }
            glm::mat4 Transform{1.0f};

            std::vector<Mesh> m_Meshes;
            std::vector<Texture2D> images;
            std::vector<uint32_t> textureIndices;
            std::vector<uint32_t> m_Ids;
            uint32_t AssetId;
    };

}  // namespace FooGame
