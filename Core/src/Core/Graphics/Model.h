#pragma once
#include "Core/Backend/Vertex.h"
#include "Core/Core/Base.h"
#include "Core/Graphics/Texture2D.h"
namespace FooGame
{

    class Mesh
    {
        public:
            Mesh(List<Vertex>&& vertices, List<u32>&& indices);
            List<Vertex> m_Vertices;
            List<u32> m_Indices;

            void SetTexture(const Shared<Texture2D>& texture)
            {
                m_Texture = texture;
            }

            VkDescriptorSetLayout GetLayout() const { return m_Layout; }
            VkDescriptorSet* GetSet(u32 index)
            {
                return &m_DescriptorSets[index];
            }
            List<VkDescriptorSet>& GetSets() { return m_DescriptorSets; }
            void UpdateDescriptorBuffer(const VkBuffer& buffer, VkDevice device,
                                        u32 index);
            List<VkDescriptorSet> m_DescriptorSets{3};
            VkDescriptorSetLayout m_Layout;
            Shared<Texture2D> m_Texture;
    };
    class Model
    {
        public:
            static Shared<Model> LoadModel(const String& path);
            List<Mesh>& GetMeshes() { return m_Meshes; }
            Model(List<Mesh>&& meshes);
            void SetId(u32 id) { m_Id = id; }
            const u32 GetId() const { return m_Id; }

        private:
            List<Mesh> m_Meshes;
            u32 m_Id = 0;
    };

}  // namespace FooGame
