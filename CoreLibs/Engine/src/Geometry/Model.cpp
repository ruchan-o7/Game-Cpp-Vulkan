#include "Model.h"
#include "../Engine/Api.h"
#include "../Engine/Device.h"
#include "../Engine/VulkanCheckResult.h"
namespace FooGame
{
    Model::Model(std::vector<Mesh>&& meshes) : m_Meshes(std::move(meshes))
    {
    }
    Mesh::Mesh(const std::vector<Vertex>& vertices,
               const std::vector<uint32_t>& indices)
        : m_Vertices(vertices), m_Indices(indices)
    {
    }
    Mesh::~Mesh()
    {
        m_Vertices.clear();
        m_Indices.clear();
    }
    Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
        : m_Vertices(std::move(vertices)), m_Indices(std::move(indices))
    {
        auto device = Api::GetDevice();
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding            = 0;
        uboLayoutBinding.descriptorCount    = 1;
        uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding         = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers  = nullptr;
        samplerLayoutBinding.stageFlags          = VK_SHADER_STAGE_FRAGMENT_BIT;
        VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding,
                                                    samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = ARRAY_COUNT(bindings);
        layoutInfo.pBindings    = bindings;

        VK_CALL(vkCreateDescriptorSetLayout(device->GetDevice(), &layoutInfo,
                                            nullptr, &m_Layout));
    }
    void Mesh::UpdateDescriptorBuffer(const VkBuffer& buffer, VkDevice device,
                                      uint32_t index)
    {
        // for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrites[2] = {};
        descriptorWrites[0].sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet     = m_DescriptorSets[index];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo     = &bufferInfo;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = m_Texture->ImageView;
        imageInfo.sampler     = m_Texture->Sampler;

        descriptorWrites[1].sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet     = m_DescriptorSets[index];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets(device, ARRAY_COUNT(descriptorWrites),
                               descriptorWrites, 0, nullptr);
    }

}  // namespace FooGame
