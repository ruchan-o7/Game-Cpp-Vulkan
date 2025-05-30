#define VOLK_IMPLEMENTATION
#include "Volk/volk.h"
#include "Renderer.h"

#include <GLFW/glfw3.h>
#include <fstream>
#include <ios>
#include <memory>
#include <stdexcept>
#include "../Core/Assert.h"
#include "../Core/Log.h"
#include "VulkanDebug.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanShader.h"

namespace fg {

const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation",
};

std::shared_ptr<Renderer> Renderer::Create(GLFWwindow* window, const VkAllocationCallbacks* acb) {
  if (volkInitialize() != VK_SUCCESS) {
    return nullptr;
  }
  VkInstance vkInstance = VK_NULL_HANDLE;
  VkPhysicalDevice vkPDevice = VK_NULL_HANDLE;

  VkApplicationInfo appInfo {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.pApplicationName = "Foo game engine";
  appInfo.pEngineName = "FG Engine";
  appInfo.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo pCreateInfo {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  pCreateInfo.pApplicationInfo = &appInfo;

  uint32_t extCount = 0;
  const char** glfwExtensions = nullptr;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&extCount);
  std::vector<const char*> instanceExtensions;
  instanceExtensions.reserve(extCount + 1);
  for (int i = 0; i < extCount; i++) {
    instanceExtensions.emplace_back(glfwExtensions[i]);
  }
  instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  pCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
  pCreateInfo.enabledExtensionCount = instanceExtensions.size();
  pCreateInfo.ppEnabledLayerNames = validationLayers;
  pCreateInfo.enabledLayerCount = 1;

  VkResult res = vkCreateInstance(&pCreateInfo, acb, &vkInstance);

  volkLoadInstance(vkInstance);

  if (res != VK_SUCCESS) {
    throw std::runtime_error("Can not create vulkan instance");
  }

  auto Instance = std::make_shared<VulkanInstance>(vkInstance, acb);

  auto messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  auto messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  SetupDebugUtils(vkInstance, messageSeverity, messageType, 0, nullptr);

  VkPhysicalDevice physicalDevices[8];
  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
  vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices);

  for (uint32_t i = 0; i < physicalDeviceCount; i++) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &props);
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      vkPDevice = physicalDevices[i];
    }
  }
  auto physicalDevice = std::make_unique<VulkanPhysicalDevice>(vkPDevice);

  Renderer* renderer = new Renderer(window, Instance, std::move(physicalDevice), acb);

  return std::shared_ptr<Renderer>(renderer);
}

void Renderer::WaitGPU() const {
  m_LogicalDevice->WaitIdle();
}
Renderer::Renderer(GLFWwindow* window, const std::shared_ptr<VulkanInstance>& instance,
                   std::unique_ptr<VulkanPhysicalDevice> pDevice,
                   const VkAllocationCallbacks* alloc)
    : m_Instance(instance),
      m_Window(window),
      m_PhysicalDevice(std::move(pDevice)),
      m_AllocCB(alloc) {
  uint32_t queueIndex = m_PhysicalDevice->GetQueuFamilyIndices(VK_QUEUE_GRAPHICS_BIT);

  VkDeviceQueueCreateInfo queueInfo {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  queueInfo.queueFamilyIndex = queueIndex;
  queueInfo.queueCount = 1;
  float priority[] = {1.0f};
  queueInfo.pQueuePriorities = priority;

  auto pDev = m_PhysicalDevice->GetHandle();

  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(pDev, &features);

  const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                              VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};

  VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRendering {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
      VK_NULL_HANDLE,
      VK_TRUE,
  };

  VkDeviceCreateInfo info {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  info.pQueueCreateInfos = &queueInfo;
  info.queueCreateInfoCount = 1;
  info.pEnabledFeatures = &features;
  info.enabledExtensionCount = 2;
  info.ppEnabledExtensionNames = extensions;
  info.enabledLayerCount = 1;
  info.ppEnabledLayerNames = validationLayers;
  info.pNext = &dynamicRendering;

  VkDevice device = VK_NULL_HANDLE;
  VkResult res = vkCreateDevice(pDev, &info, m_AllocCB, &device);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Can not create logical device");
  }
  volkLoadDevice(device);
  m_LogicalDevice = std::make_shared<VulkanLogicalDevice>(device, queueIndex, m_AllocCB);
  m_VkQueue = m_LogicalDevice->GetQueue();
  m_Swapchain = std::make_shared<VulkanSwapchain>(m_Window, this, m_Instance, m_LogicalDevice,
                                                  *m_PhysicalDevice);
  VkCommandPoolCreateInfo cmdPool {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  cmdPool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  m_CmdPool = m_LogicalDevice->CreateCommandPool(cmdPool);

  VkCommandBufferAllocateInfo allocInfo {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = m_CmdPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;
  m_Cmd = m_LogicalDevice->AllocateCmdBuffer(allocInfo);
}

void Renderer::BeginRendering() {
  auto cmd = GetCurrentCmdBuffer();
  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo info {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  vkBeginCommandBuffer(cmd, &info);

  VkRenderingInfoKHR beginInfo {VK_STRUCTURE_TYPE_RENDERING_INFO, 0};
  beginInfo.renderArea = {
      {0, 0},
      m_Swapchain->GetExtent()
  };
  beginInfo.layerCount = 1;

  VkRenderingAttachmentInfo colorInfo {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR};
  colorInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorInfo.imageView = m_Swapchain->GetCurrentImageView();
  colorInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorInfo.clearValue = {
      {0.2f, 0.2f, 0.2f, 1.0f}
  };
  std::vector<VkRenderingAttachmentInfo> colorAttachments;
  colorAttachments.push_back(colorInfo);

  beginInfo.colorAttachmentCount = colorAttachments.size();
  beginInfo.pColorAttachments = colorAttachments.data();

  vkCmdBeginRendering(cmd, &beginInfo);
}

void Renderer::EndRendering() {
  auto cmd = GetCurrentCmdBuffer();
  vkCmdEndRenderingKHR(cmd);
  vkEndCommandBuffer(cmd);
}
void Renderer::BindPipeline(const Ref<VulkanGraphicsPipeline>& pipeline) {
  m_CurrentPipeline = pipeline;
  vkCmdBindPipeline(GetCurrentCmdBuffer(),VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline->GetHandle());
}
void Renderer::Draw(const DrawAttributes& attribs) {
  FOO_ASSERT(m_CurrentPipeline != nullptr);
  vkCmdDraw(GetCurrentCmdBuffer(), attribs.VertexCount, attribs.InstanceCount, attribs.FirstVertex,
            attribs.FirstInstance);
}

VkResult Renderer::Flush(const std::function<VkResult(VkQueue, VkCommandBuffer)>& func) {
  return func(m_VkQueue, GetCurrentCmdBuffer());
}
void Renderer::Present(VkPresentInfoKHR& info) {
  vkQueuePresentKHR(m_VkQueue, &info);
}

Ref<VulkanShader> Renderer::CreateShader(const ShaderDescription& desc) const {
  FOO_ASSERT(desc.Stage != 0)
  FOO_ASSERT(!desc.EntryPoint.empty());
  Buffer buff;
  if (desc.ByteCode) {
    buff = desc.ByteCode;
  } else {
    std::ifstream in {desc.Path, std::ios::ate | std::ios::binary};
    if (!std::filesystem::exists(desc.Path)) {
      auto formatted = fmt::format("Can not find: '{}' does not exists!", desc.Path.string());
      FOO_CORE_ERROR(formatted);
      return nullptr;
    }
    if (!in.is_open()) {
      FOO_CORE_ERROR("Can not open file: {}", desc.Path.string());
      return nullptr;
    }
    size_t size = in.tellg();
    if (size == 0) {
      FOO_CORE_ERROR("Failed to read: '{}' does not exists!", desc.Path.string());
      return nullptr;
    }
    in.seekg(0);
    buff.Allocate(size);
    in.read(buff.As<char>(), size);
  }
  VkShaderModuleCreateInfo info {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  info.pCode = buff.As<uint32_t>();
  info.codeSize = buff.Size;
  auto handle = m_LogicalDevice->CreateShader(info, desc.Name);
  if (handle) {
    return MakeRef<VulkanShader>(desc, std::move(handle));
  }

  FOO_CORE_ERROR("Can not create shader handle: Name: {}", desc.Name != nullptr ? desc.Name : "");
  return nullptr;
}
Ref<VulkanGraphicsPipeline> Renderer::CreateGraphicsPipeline(
    const GraphicsPipelineDescription& desc) {
  VkPipelineDynamicStateCreateInfo dynamicState {
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicState.pDynamicStates = desc.DynamicStates.data();
  dynamicState.dynamicStateCount = (uint32_t)desc.DynamicStates.size();

  // TODO:
  VkPipelineVertexInputStateCreateInfo vertexInput {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vertexInput.vertexBindingDescriptionCount = 0;
  vertexInput.vertexAttributeDescriptionCount = 0;
  vertexInput.pVertexAttributeDescriptions = 0;
  vertexInput.pVertexBindingDescriptions = 0;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  inputAssembly.topology = desc.Topology;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;  // TODO:
  viewport.width = (float)m_Swapchain->GetExtent().width;
  viewport.height = (float)m_Swapchain->GetExtent().height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor {};
  scissor.offset = {0, 0};
  scissor.extent = m_Swapchain->GetExtent();

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

  VkPipelineLayoutCreateInfo layoutInfo {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutInfo.setLayoutCount = 0;             // Optional
  layoutInfo.pSetLayouts = nullptr;          // Optional
  layoutInfo.pushConstantRangeCount = 0;     // Optional
  layoutInfo.pPushConstantRanges = nullptr;  // Optional
  auto layout = m_LogicalDevice->CreatePipelineLayout(layoutInfo);

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
  pipelineInfo.layout = layout;
  pipelineInfo.renderPass = 0;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = 0;
  pipelineInfo.basePipelineIndex = -1;

  const auto swapchainFormat = m_Swapchain->Format();

  const VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info {
      VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
      VK_NULL_HANDLE,
      0,
      1,
      &swapchainFormat.format,
  };
  pipelineInfo.pNext = &pipeline_rendering_create_info;

  pipelineInfo.pStages = shaderStages.data();
  auto pipeline = m_LogicalDevice->CreateGraphicsPipeline(pipelineInfo);
  return MakeRef<VulkanGraphicsPipeline>(std::move(pipeline), std::move(layout));
}

void Renderer::Destroy() {
}

}  // namespace fg
