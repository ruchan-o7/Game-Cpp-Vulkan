#include "Mesh.h"
#include "../Defines.h"
#include "../Engine/VulkanCheckResult.h"
#include "../Engine/Backend.h"
#include "../Core/RenderDevice.h"
namespace FooGame
{

    static void CreateLayout(VkDescriptorSetLayout& layout)
    {
        auto device = Backend::GetRenderDevice()->GetVkDevice();  // Api::GetVkDevice();
        // auto device = Api::GetVkDevice();
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding            = 0;
        uboLayoutBinding.descriptorCount    = 1;
        uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding             = 1;
        samplerLayoutBinding.descriptorCount     = 1;
        samplerLayoutBinding.descriptorType      = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers  = nullptr;
        samplerLayoutBinding.stageFlags          = VK_SHADER_STAGE_FRAGMENT_BIT;
        VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = ARRAY_COUNT(bindings);
        layoutInfo.pBindings    = bindings;

        VK_CALL(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout));
    }
    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
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
        if (m_Layout)
        {
            auto device = Backend::GetRenderDevice()->GetVkDevice();  // Api::GetVkDevice();
            // auto device = Api::GetVkDevice();
            vkDestroyDescriptorSetLayout(device, m_Layout, nullptr);
        }
    }
    Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
        : m_Vertices(std::move(vertices)), m_Indices(std::move(indices))
    {
        CreateLayout(m_Layout);
    }
    Mesh::Mesh(Mesh&& other)
    {
        m_Vertices       = std::move(other.m_Vertices);
        m_Indices        = std::move(other.m_Indices);
        m_DescriptorSets = std::move(other.m_DescriptorSets);
        m_Layout         = std::move(other.m_Layout);
        // m_Texture        = std::move(other.m_Texture);
        MeshPrimitives = std::move(other.MeshPrimitives);
        materialData   = std::move(other.materialData);
        RenderId       = other.RenderId;
        other.m_Vertices.clear();
        other.m_Indices.clear();
        other.m_DescriptorSets.clear();
        other.m_Layout = nullptr;
        // other.m_Texture      = nullptr;
        other.MeshPrimitives = {};
        other.materialData   = {};
        other.RenderId       = -1;
    }

}  // namespace FooGame
