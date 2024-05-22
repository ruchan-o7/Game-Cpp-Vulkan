#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "../Engine/Texture2D.h"
#include "../Geometry/Vertex.h"
namespace FooGame
{

    class Mesh
    {
        public:
            Mesh(std::vector<Vertex>&& vertices,
                 std::vector<uint32_t>&& indices);
            Mesh(Mesh&&) = default;

            std::vector<Vertex> m_Vertices;
            std::vector<uint32_t> m_Indices;

            void SetTexture(const std::shared_ptr<Texture2D>& texture)
            {
                m_Texture = texture;
            }

            VkDescriptorSetLayout GetLayout() const { return m_Layout; }
            VkDescriptorSet* GetSet(uint32_t index)
            {
                return &m_DescriptorSets[index];
            }
            std::vector<VkDescriptorSet>& GetSets() { return m_DescriptorSets; }
            void UpdateDescriptorBuffer(const VkBuffer& buffer, VkDevice device,
                                        uint32_t index);
            std::vector<VkDescriptorSet> m_DescriptorSets{3};
            VkDescriptorSetLayout m_Layout;
            std::shared_ptr<Texture2D> m_Texture;
    };
    class Model
    {
        public:
            Model(std::vector<Mesh>&& meshes);
            std::vector<Mesh>& GetMeshes() { return m_Meshes; }
            void PushId(uint32_t id) { m_Ids.push_back(id); }
            const std::vector<uint32_t>& GetIds() const { return m_Ids; }
            glm::mat4 Transform{1.0f};

        private:
            std::vector<Mesh> m_Meshes;
            std::vector<uint32_t> m_Ids;
    };

}  // namespace FooGame
