#pragma once
#include "VulkanObject.h"
#include "Misc.h"
#include "VulkanDescriptorSet.h"

#include "../Core/Ref.h"
#include "../Core/Assert.h"

namespace fg {

class VulkanShader;

struct SamplerInfo {
    Filter MagFilter = Filter::Linear;
    Filter MinFilter = Filter::Linear;
    SamplerAddressMode AddressMode = SamplerAddressMode::Repeat;
    bool EnableAnisotropy = true;
    SamplerBorderColor BorderColor = SamplerBorderColor::IntTransparentBlack;
    bool Compare = false;
    CompareOperation CompOp = CompareOperation::Always;
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
    std::vector<ShaderVariableDesc> ShaderVariables;
    VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
    float LineWidth = 1.0f;
    VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    VkFormat RenderTargetFormat = VK_FORMAT_UNDEFINED;
    SamplerInfo Sampler;
};

class VulkanGraphicsPipeline : public RefBase {
  public:
    VulkanGraphicsPipeline(const GraphicsPipelineDescription& desc, Renderer* renderer);
    virtual ~VulkanGraphicsPipeline() = default;

    VkPipeline GetHandle() const {
      return m_Pipeline;
    }

    VkPipelineLayout Layout() const {
      return m_Layout;
    }

    VkSampler GetSampler() const {
      return m_Sampler;
    }
    VkDescriptorPool GetPool() const {
      return m_DescriptorPool;
    }
    const GraphicsPipelineDescription& GetDesc() const {
      return m_Desc;
    }

    VkDescriptorSetLayout DescriptorSetLayout() const {
      FOO_ASSERT(m_DescriptorLayouts.size() > 0);
      return m_DescriptorLayouts[0];
    }
    const std::vector<DescriptorSetLayoutWrapper>& GetSetLayouts() {
      return m_DescriptorLayouts;
    }

    // VkDescriptorSet CreateDescriptorSet();
    Ref<VulkanDescriptorSet> CreateDescriptorSet();

  private:
    GraphicsPipelineDescription m_Desc;
    Renderer* m_Renderer;

    DescriptorPoolWrapper m_DescriptorPool;
    PipelineWrapper m_Pipeline;
    PipelineLayoutWrapper m_Layout;
    SamplerWrapper m_Sampler;
    std::vector<DescriptorSetLayoutWrapper> m_DescriptorLayouts;
};

}  // namespace fg
