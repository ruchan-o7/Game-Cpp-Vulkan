#include "src/Core/Ref.h"
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
}

void Renderer::CreateDeviceAndSwapchain() {
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
  m_LogicalDevice = std::make_shared<VulkanLogicalDevice>(device, queueIndex, GetPtr(), m_AllocCB);
  m_VkQueue = m_LogicalDevice->GetQueue();
  m_Swapchain = std::make_shared<VulkanSwapchain>(m_Window, GetPtr(), m_Instance, m_LogicalDevice,
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

Ref<VulkanImage> Renderer::CreateImage(const ImageDescription& desc, VkImage handle) {
  return nullptr;
}

Ref<VulkanBuffer> Renderer::CreateBuffer(const BufferDescription& desc) {
  FOO_ASSERT(desc.Usage != BufferUsage::None);

  if (desc.Usage == BufferUsage::Index || desc.Usage == BufferUsage::Vertex) {
    FOO_ASSERT(desc.Size > 0, "Vertex and Index buffers must be provide data");
  }

  auto buffer = MakeRef<VulkanBuffer>(desc, GetPtr());
  return buffer;
}

void Renderer::BeginRendering() {
  auto cmd = GetCurrentCmdBuffer();
  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo info {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  vkBeginCommandBuffer(cmd, &info);
  {
    VkImageMemoryBarrier barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.image = m_Swapchain->GetCurrentImage();
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr,
                         1, &barrier);
  }

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
  VkRect2D scissor {
      {0, 0},
      m_Swapchain->GetExtent()
  };
  vkCmdSetScissor(cmd, 0, 1, &scissor);
  VkViewport vp {
      0,    0,   (float)m_Swapchain->GetExtent().width, (float)m_Swapchain->GetExtent().height,
      0.0f, 1.0f};
  vkCmdSetViewport(cmd, 0, 1, &vp);
}

void Renderer::EndRendering() {
  auto cmd = GetCurrentCmdBuffer();
  vkCmdEndRenderingKHR(cmd);
  {
    VkImageMemoryBarrier barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.image = m_Swapchain->GetCurrentImage();
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);
  }
  vkEndCommandBuffer(cmd);
}
void Renderer::BindPipeline(const Ref<VulkanGraphicsPipeline>& pipeline) {
  m_CurrentPipeline = pipeline;
  vkCmdBindPipeline(GetCurrentCmdBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetHandle());
}
void Renderer::Draw(const DrawAttributes& attribs) {
  FOO_ASSERT(m_CurrentPipeline != nullptr);
  vkCmdDraw(GetCurrentCmdBuffer(), attribs.VertexCount, attribs.InstanceCount, attribs.FirstVertex,
            attribs.FirstInstance);
}

VkResult Renderer::Flush(const std::function<VkResult(VkQueue, VkCommandBuffer)>& func) {
  return func(m_VkQueue, GetCurrentCmdBuffer());
}
VkResult Renderer::Present(VkPresentInfoKHR& info) {
  return vkQueuePresentKHR(m_VkQueue, &info);
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
  FOO_ASSERT(desc.RenderTargetFormat != VK_FORMAT_UNDEFINED);
  return MakeRef<VulkanGraphicsPipeline>(desc, GetPtr());
}

void Renderer::Destroy() {
  WaitGPU();
}

}  // namespace fg
