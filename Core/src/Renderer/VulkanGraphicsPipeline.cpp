#include "VulkanGraphicsPipeline.h"
#include "../Core/Assert.h"
#include "Renderer.h"

namespace {
static VkFormat ToVk(fg::ValueType type) {
  switch (type) {
    case fg::VT_FLOAT:
      return VK_FORMAT_R32_SFLOAT;
    case fg::VT_VEC2:
      return VK_FORMAT_R32G32_SFLOAT;
    case fg::VT_VEC3:
      return VK_FORMAT_R32G32B32_SFLOAT;
    case fg::VT_VEC4:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
      break;
  }
}

static VkVertexInputRate ToVk(fg::VertexInputRate rate) {
  switch (rate) {
    case fg::VertexInputRate::Vertex:
      return VK_VERTEX_INPUT_RATE_VERTEX;
    case fg::VertexInputRate::Instance:
      return VK_VERTEX_INPUT_RATE_INSTANCE;
      break;
  }
}

}  // namespace

namespace fg {

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineDescription& desc,
                                               std::weak_ptr<Renderer> renderer)
    : m_Desc(desc), m_Renderer(renderer) {
  FOO_ASSERT(!renderer.expired());
  auto device = m_Renderer.lock()->GetLogicalDevice();

  VkPipelineDynamicStateCreateInfo dynamicState {
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicState.pDynamicStates = desc.DynamicStates.data();
  dynamicState.dynamicStateCount = (uint32_t)desc.DynamicStates.size();

  VkPipelineVertexInputStateCreateInfo vertexInput {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  std::vector<VkVertexInputAttributeDescription> attributesDescs;
  std::vector<VkVertexInputBindingDescription> bindingDescs;
  uint32_t i = 0;
  uint32_t offset = 0;
  VkVertexInputAttributeDescription attrDesc {};
  for (const auto& attr : m_Desc.VertexAttributes) {
    attrDesc.binding = attr.Binding;
    attrDesc.location = i;
    attrDesc.format = ToVk(attr.Type);
    attrDesc.offset = offset;
    attributesDescs.push_back(attrDesc);

    offset += (uint8_t)attr.Type;
    i++;
  }
  for (const auto& elem : m_Desc.BindingDescs) {
    VkVertexInputBindingDescription desc {};
    desc.binding = elem.Binding;
    desc.stride = elem.Stride;
    desc.inputRate = ToVk(elem.Rate);
  }

  vertexInput.vertexBindingDescriptionCount = (uint32_t)bindingDescs.size();
  vertexInput.pVertexBindingDescriptions = bindingDescs.data();

  vertexInput.vertexAttributeDescriptionCount = (uint32_t)attributesDescs.size();
  vertexInput.pVertexAttributeDescriptions = attributesDescs.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  inputAssembly.topology = desc.Topology;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
  VkRect2D scissor {
      {0, 0},
      {1, 1}
  };

  VkPipelineViewportStateCreateInfo viewportState {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;
  viewportState.pViewports = &viewport;

  VkPipelineRasterizationStateCreateInfo rasterizer {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = desc.PolygonMode;
  rasterizer.lineWidth = desc.LineWidth;
  rasterizer.cullMode = desc.CullMode;
  rasterizer.frontFace = desc.FrontFace;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;           // Optional
  multisampling.pSampleMask = nullptr;             // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
  multisampling.alphaToOneEnable = VK_FALSE;       // Optional

  VkPipelineColorBlendAttachmentState colorBlendAttachment {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

  VkPipelineColorBlendStateCreateInfo colorBlending {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;  // Optional
  colorBlending.blendConstants[1] = 0.0f;  // Optional
  colorBlending.blendConstants[2] = 0.0f;  // Optional
  colorBlending.blendConstants[3] = 0.0f;  // Optional
  std::vector<VkPushConstantRange> pcranges;
  for (const auto& pc : m_Desc.PushConstants) {
    VkPushConstantRange range {pc.ShaderStage, pc.Offset, pc.Size};
    pcranges.emplace_back(range);
  }

  VkPipelineLayoutCreateInfo layoutInfo {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutInfo.setLayoutCount = 0;     // Optional
  layoutInfo.pSetLayouts = nullptr;  // Optional
  layoutInfo.pushConstantRangeCount = (uint32_t)pcranges.size();
  layoutInfo.pPushConstantRanges = pcranges.data();
  m_Layout = device->CreatePipelineLayout(layoutInfo, m_Desc.Name);

  VkGraphicsPipelineCreateInfo pipelineInfo {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

  std::vector<VulkanShader*> shaders;
  if (desc.VertexShader) {
    shaders.emplace_back(desc.VertexShader);
  }
  if (desc.FragmentShader) {
    shaders.emplace_back(desc.FragmentShader);
  }
  pipelineInfo.stageCount = shaders.size();

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  shaderStages.reserve(shaders.size());

  VkPipelineShaderStageCreateInfo shaderInfo {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  for (const auto* shader : shaders) {
    shaderInfo.stage = shader->GetStage();
    shaderInfo.module = shader->GetHandle();
    shaderInfo.pName = shader->EntryPoint();
    shaderStages.emplace_back(shaderInfo);
  }

  pipelineInfo.pVertexInputState = &vertexInput;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = nullptr;  // Optional
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = m_Layout;
  pipelineInfo.renderPass = 0;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = 0;
  pipelineInfo.basePipelineIndex = -1;

  const VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info {
      VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
      VK_NULL_HANDLE,
      0,
      1,
      &m_Desc.RenderTargetFormat,
  };
  pipelineInfo.pNext = &pipeline_rendering_create_info;

  pipelineInfo.pStages = shaderStages.data();

  m_Pipeline = device->CreateGraphicsPipeline(pipelineInfo, m_Desc.Name);
}

}  // namespace fg
