#pragma once
#include "VulkanLogicalDevice.h"
#include "VulkanObject.h"
#include "../Core/Ref.h"

namespace fg {

class VulkanShader;

struct VertexInputLayout {
    int placeholderForNow;
};
struct GraphicsPipelineDescription {
    const char* Name = nullptr;
    VulkanShader* VertexShader = nullptr;
    VulkanShader* FragmentShader = nullptr;
    std::vector<VkDynamicState> DynamicStates;
    std::vector<VertexInputLayout> VertexLayout;
    VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
    float LineWidth = 1.0f;
    VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace FrontFace = VK_FRONT_FACE_CLOCKWISE;
};

class VulkanGraphicsPipeline : public RefBase {
  public:
    VulkanGraphicsPipeline(PipelineWrapper pipeline, PipelineLayoutWrapper layout)
        : m_Pipeline(std::move(pipeline)), m_Layout(std::move(layout)) {
    }
    virtual ~VulkanGraphicsPipeline() = default;

  private:
    PipelineWrapper m_Pipeline;
    PipelineLayoutWrapper m_Layout;
};

}  // namespace fg
