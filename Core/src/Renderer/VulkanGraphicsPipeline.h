#pragma once
#include "VulkanObject.h"
#include "../Core/Ref.h"
#include "../Core/Assert.h"

namespace fg {

class VulkanShader;

enum ValueType : uint8_t {
  VT_FLOAT = sizeof(float),
  VT_VEC2 = VT_FLOAT * 2,
  VT_VEC3 = VT_FLOAT * 3,
  VT_VEC4 = VT_FLOAT * 4,

  VT_UINT = sizeof(uint32_t),
  VT_IVEC2 = VT_UINT * 2,
  VT_IVEC3 = VT_UINT * 3,
  VT_IVEC4 = VT_UINT * 4,

  VT_DOUBLE = sizeof(double),
};

enum class VertexInputRate : uint8_t {
  Vertex = 0,
  Instance,
};

// Vertex input binding and attribute description
struct VertexAttribute {
    uint8_t Binding = 0, Location = 0;
    ValueType Type;
};

struct ShaderVariable {
    uint32_t Binding = 0;
    // UNIFORM
    VkDescriptorType Type;
    uint32_t Count = 1;
    //= VK_SHADER_STAGE_VERTEX_BIT
    VkShaderStageFlagBits ShaderStage;
};

struct VertexInputBindingDesc {
    uint32_t Binding = 0;
    uint32_t Stride = 0;
    VertexInputRate Rate = VertexInputRate ::Vertex;
};
struct PushConstantRange {
    VkShaderStageFlags ShaderStage;
    uint32_t Offset = 0, Size = 0;
};

struct GraphicsPipelineDescription {
    const char* Name = nullptr;
    VulkanShader* VertexShader = nullptr;
    VulkanShader* FragmentShader = nullptr;
    std::vector<VkDynamicState> DynamicStates;
    std::vector<VertexAttribute> VertexAttributes;
    std::vector<VertexInputBindingDesc> BindingDescs;
    std::vector<PushConstantRange> PushConstants;
    std::vector<ShaderVariable> ShaderVariables;
    VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
    float LineWidth = 1.0f;
    VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    VkFormat RenderTargetFormat = VK_FORMAT_UNDEFINED;
};

class VulkanGraphicsPipeline : public RefBase {
  public:
    VulkanGraphicsPipeline(const GraphicsPipelineDescription& desc,
                           const std::shared_ptr<VulkanLogicalDevice>& renderer);
    virtual ~VulkanGraphicsPipeline() = default;

    VkPipeline GetHandle() const {
      return m_Pipeline;
    }

    VkPipelineLayout Layout() const {
      return m_Layout;
    }

    VkDescriptorSetLayout DescriptorSetLayout() const {
      FOO_ASSERT(m_DescriptorLayouts.size() > 0);
      return m_DescriptorLayouts[0];
    }

    VkDescriptorSet CreateDescriptorSet();

  private:
    GraphicsPipelineDescription m_Desc;
    std::shared_ptr<VulkanLogicalDevice> m_LogicalDevice;

    DescriptorPoolWrapper m_DescriptorPool;
    PipelineWrapper m_Pipeline;
    PipelineLayoutWrapper m_Layout;
    std::vector<DescriptorSetLayoutWrapper> m_DescriptorLayouts;
};

}  // namespace fg
