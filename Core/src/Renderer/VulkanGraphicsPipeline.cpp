#include "VulkanGraphicsPipeline.h"
#include "VulkanShader.h"
#include "Renderer.h"
#include "VulkanPhysicalDevice.h"
#include "TypeConversions.h"

namespace fg {

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineDescription& desc,
                                               Renderer* renderer)
    : m_Desc(desc), m_Renderer(renderer) {
  auto device = renderer->GetLogicalDevice();

  VkPipelineDynamicStateCreateInfo dynamicState {
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicState.pDynamicStates = desc.DynamicStates.data();
  dynamicState.dynamicStateCount = (uint32_t)desc.DynamicStates.size();

  VkPipelineVertexInputStateCreateInfo vertexInput {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  std::vector<VkVertexInputAttributeDescription> attributesDescs;
  std::vector<VkVertexInputBindingDescription> bindingDescs;

  uint32_t offset = 0;

  for (const auto& attr : m_Desc.VertexAttributes) {
    VkVertexInputAttributeDescription attrDesc {};
    attrDesc.binding = attr.Binding;
    attrDesc.location = attr.Location;
    attrDesc.format = ToVk(attr.Type);
    attrDesc.offset = offset;
    attributesDescs.push_back(attrDesc);

    offset += (uint8_t)attr.Type;
  }
  for (const auto& elem : m_Desc.BindingDescs) {
    VkVertexInputBindingDescription bindingDesc {};
    bindingDesc.binding = elem.Binding;
    bindingDesc.stride = offset;
    bindingDesc.inputRate = ToVk(elem.Rate);
    bindingDescs.push_back(bindingDesc);
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

  std::map<uint32_t, std::vector<ShaderVariableDesc>> groped;
  for (const auto var : m_Desc.ShaderVariables) {
    groped[var.Set].push_back(var);
  }
  std::vector<VkDescriptorSetLayout> setLayouts;
  for (const auto& [setIdx, vars] : groped) {
    std::vector<VkDescriptorSetLayoutBinding> dSetBinding;
    VkDescriptorSetLayoutBinding binding {};
    for (const auto& var : vars) {
      binding.binding = var.Binding;
      binding.descriptorType = ToVk(var.Type);
      binding.descriptorCount = var.Count;
      binding.stageFlags = var.ShaderStages;
      binding.pImmutableSamplers = VK_NULL_HANDLE;  // TODO:
      dSetBinding.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo setLayoutInfo {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    setLayoutInfo.pBindings = dSetBinding.data();
    setLayoutInfo.bindingCount = (uint32_t)dSetBinding.size();
    setLayouts.emplace_back(
        m_DescriptorLayouts.emplace_back(device->CreateDescriptorSetLayout(setLayoutInfo)));
  }

  VkPipelineLayoutCreateInfo layoutInfo {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutInfo.setLayoutCount = (uint32_t)setLayouts.size();
  layoutInfo.pSetLayouts = setLayouts.data();  // Optional
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

  const VkDescriptorPoolSize poolSizes[] = {
      {        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
  };

  VkDescriptorPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  poolInfo.poolSizeCount = 2;
  poolInfo.pPoolSizes = poolSizes;
  poolInfo.maxSets = 100;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  m_DescriptorPool = device->CreateDescriptorPool(poolInfo);

  VkSamplerCreateInfo samplerInfo {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  samplerInfo.magFilter = ToVk(m_Desc.Sampler.MagFilter);
  samplerInfo.minFilter = ToVk(m_Desc.Sampler.MinFilter);
  samplerInfo.addressModeU = ToVk(m_Desc.Sampler.AddressMode);
  samplerInfo.addressModeV = ToVk(m_Desc.Sampler.AddressMode);
  samplerInfo.addressModeW = ToVk(m_Desc.Sampler.AddressMode);
  samplerInfo.anisotropyEnable = m_Desc.Sampler.EnableAnisotropy;
  samplerInfo.maxAnisotropy =
      renderer->GetPhysicalDevice().GetProperties().limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = ToVk(m_Desc.Sampler.BorderColor);
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = m_Desc.Sampler.Compare;
  samplerInfo.compareOp = ToVk(m_Desc.Sampler.CompOp);
  // TODO:
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;
  m_Sampler = device->CreateSampler(samplerInfo, m_Desc.Name);
}

Ref<VulkanDescriptorSet> VulkanGraphicsPipeline::CreateDescriptorSet() {
  return MakeRef<VulkanDescriptorSet>(this, m_Renderer);
}

}  // namespace fg
