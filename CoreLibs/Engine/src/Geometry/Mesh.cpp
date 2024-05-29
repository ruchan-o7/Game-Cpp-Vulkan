#include "Mesh.h"
#include "../Defines.h"
#include "../Engine/Api.h"
#include "../Engine/Device.h"
#include "../Engine/VulkanCheckResult.h"
namespace FooGame
{

    static void CreateLayout(VkDescriptorSetLayout& layout)
    {
        auto device = Api::GetVkDevice();
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

        VK_CALL(
            vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout));
    }
    Mesh::Mesh(const std::vector<Vertex>& vertices,
               const std::vector<uint32_t>& indices)
        : m_Vertices(vertices), m_Indices(indices)
    {
        CreateLayout(m_Layout);
    }
    Mesh::Mesh()
    {
        CreateLayout(m_Layout);
    }
    Mesh::~Mesh()
    {
        m_Vertices.clear();
        m_Indices.clear();
    }
    Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
        : m_Vertices(std::move(vertices)), m_Indices(std::move(indices))
    {
        CreateLayout(m_Layout);
    }

}  // namespace FooGame
