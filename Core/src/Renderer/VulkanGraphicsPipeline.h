#pragma once
#include "VulkanObject.h"
#include "../Core/Ref.h"

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
    uint8_t Binding;
    ValueType Type;
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
    VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
    float LineWidth = 1.0f;
    VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace FrontFace = VK_FRONT_FACE_CLOCKWISE;
    VkFormat RenderTargetFormat = VK_FORMAT_UNDEFINED;
};

class VulkanGraphicsPipeline : public RefBase {
  public:
    VulkanGraphicsPipeline(const GraphicsPipelineDescription& desc,
                           std::weak_ptr<Renderer> renderer);
    virtual ~VulkanGraphicsPipeline() = default;

    VkPipeline GetHandle() const {
      return m_Pipeline;
    }

  private:
    GraphicsPipelineDescription m_Desc;
    std::weak_ptr<Renderer> m_Renderer;

    PipelineWrapper m_Pipeline;
    PipelineLayoutWrapper m_Layout;
};

}  // namespace fg
