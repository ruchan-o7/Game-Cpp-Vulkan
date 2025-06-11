#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include "Renderer.h"
#include "CommandPoolManager.h"
#include "VulkanCommandBufferPool.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanShader.h"
#include "TypeConversions.h"
#include "RenderContext.h"
#include "VulkanQueue.h"

#include "../Core/Ref.h"
#include "../Core/Assert.h"
#include "../Core/Log.h"

#include <GLFW/glfw3.h>
#include <fstream>
#include <ios>
#include <memory>
#include <stdexcept>
#include <cstdlib>

namespace fg {

Renderer::Renderer(ReferenceCounter* counter, RendererFactory* factory, const EngineInfo& info,
                   std::shared_ptr<VulkanInstance> instance,
                   std::unique_ptr<VulkanPhysicalDevice> physicalDevice,
                   std::shared_ptr<VulkanLogicalDevice> logicalDevice, Ref<VulkanQueue> queue)
    : RefBase(counter),
      m_Factory(factory),
      m_PhysicalDevice(std::move(physicalDevice)),
      m_Instance(instance),
      m_LogicalDevice(std::move(logicalDevice)),
      m_Queue(queue) {
  CommandPoolManager::CreateInfo poolInfo {*m_LogicalDevice, "Transient command pool", 0,
                                           VK_COMMAND_POOL_CREATE_TRANSIENT_BIT};

  m_TransientCmdPoolManager = std::make_unique<CommandPoolManager>(poolInfo);
  m_CmdPool = std::make_unique<VulkanCommandBufferPool>(
      m_LogicalDevice->GetPtr(), 0,
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

void Renderer::WaitGPU() const {
  m_LogicalDevice->WaitIdle();
}

void Renderer::CreateDeviceAndContext() {
  uint32_t queueIndex = 0;
  try {
    queueIndex = m_PhysicalDevice->GetQueuFamilyIndices(VK_QUEUE_GRAPHICS_BIT);
  } catch (const std::runtime_error& err) {
    FOO_CORE_ERROR("Can't find suitable queue family. Aborting!");
    exit(1);
  }
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

  VmaAllocatorCreateInfo allocatorInfo {};
  allocatorInfo.device = m_LogicalDevice->GetHandle();
  allocatorInfo.physicalDevice = m_PhysicalDevice->GetHandle();
  allocatorInfo.instance = m_Instance->GetHandle();
  allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

  VmaVulkanFunctions vmaFunc {};
  vmaImportVulkanFunctionsFromVolk(&allocatorInfo, &vmaFunc);
  allocatorInfo.pVulkanFunctions = &vmaFunc;

  auto res = vmaCreateAllocator(&allocatorInfo, &m_VMA);
  if (res != VK_SUCCESS) {
    FOO_CORE_ERROR("Can not initialize VMA");
  }
  m_LogicalDevice->SetVMAInstance(m_VMA);

  CommandPoolManager::CreateInfo poolInfo {*m_LogicalDevice, "Transient command pool", 0,
                                           VK_COMMAND_POOL_CREATE_TRANSIENT_BIT};

  m_TransientCmdPoolManager = std::make_unique<CommandPoolManager>(poolInfo);
  m_CmdPool = std::make_unique<VulkanCommandBufferPool>(
      m_LogicalDevice->GetPtr(), 0,
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

Ref<VulkanImage> Renderer::CreateImage(const ImageDescription& desc, const Buffer data) {
  FOO_ASSERT(desc.Width != 0);
  FOO_ASSERT(desc.Height != 0);
  return MakeRef<VulkanImage>(this, desc, data);
}

Ref<VulkanImage> Renderer::CreateImage(const ImageDescription& desc, ResourceState initialState,
                                       VkImage image) {
  FOO_ASSERT(image != VK_NULL_HANDLE);
  return MakeRef<VulkanImage>(this, desc, initialState, image);
}

Ref<VulkanBuffer> Renderer::CreateBuffer(const BufferDescription& desc, Buffer bufferData) {
  FOO_ASSERT(desc.Usage != BufferUsage::None);

  if (desc.Usage == BufferUsage::Index || desc.Usage == BufferUsage::Vertex) {
    FOO_ASSERT(desc.Size > 0, "Vertex and Index buffers must be provide data");
  }
  auto buffer = MakeRef<VulkanBuffer>(desc, this, bufferData);
  return buffer;
}

void Renderer::TransitionImageLayout(VulkanImage* image, VkImageLayout newLayout) {
  if (!image->IsInKnownState()) {
    FOO_CORE_ERROR("Can not transition image because image is in unknown state");
    return;
  }
  auto newState = VkImageLayoutToResouceState(newLayout);
  if (!image->CheckState(newState)) {
    TransitionImageState(*image, ResourceState::Unknown, newState);
  }
}
void Renderer::TransitionImageState(VulkanImage& image, ResourceState oldState,
                                    ResourceState newState) {
  if (oldState == ResourceState::Unknown) {
    if (image.IsInKnownState()) {
      oldState = image.State();
    } else {
      FOO_CORE_ERROR("Failed to transition the state of the texture");
    }
  } else {
    if (image.IsInKnownState() && image.State() != oldState) {
      FOO_CORE_ERROR("State is not match");
    }
  }
  auto vkImg = image.GetVkImage();
  VkImageSubresourceRange range {};
  range.aspectMask = 0;
  range.baseArrayLayer = 0;
  range.layerCount = VK_REMAINING_ARRAY_LAYERS;
  range.baseMipLevel = 0;
  range.levelCount = VK_REMAINING_MIP_LEVELS;

  const auto& desc = image.GetDesc();
  if (desc.Format == ImageFormat::D32) {
    range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  } else {
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }
  const bool hasWriteAccess = ResourceStateHasWriteAccess(oldState);
  const auto oldLayout = ResourceStateToVkImageLayout(oldState);
  const auto newLayout = ResourceStateToVkImageLayout(newState);
  const auto oldStages = ResourceStateFlagsToVkPipelineStageFlags(oldState);
  const auto newStages = ResourceStateFlagsToVkPipelineStageFlags(newState);
  m_Cmd.TransitionImageLayout(vkImg, oldLayout, newLayout, range, oldStages, newStages);
  image.SetState(newState);
}

void Renderer::Flush() {
  auto vkCmd = m_Cmd.Get();
  if (vkCmd != nullptr) {
    if (m_Cmd.GetState().InsideRendering) {
      m_Cmd.EndRendering();
    }
    m_Cmd.FlushBarriers();
  }
  return;
  VkSubmitInfo submit {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &vkCmd;
  submit.waitSemaphoreCount = 0;
  submit.signalSemaphoreCount = 0;
  FOO_ASSERT(false, "did not implemented");
}

void Renderer::ExecuteCommandBuffer(const VkSubmitInfo& info, VkFence* fence) {
  auto err = m_Queue->Submit(info);
  if (err != VK_SUCCESS) {
    FOO_CORE_ERROR("Can not submit queue");
  }
}
VkResult Renderer::Flush(const std::function<VkResult(VulkanQueue*)>& func) {
  auto res = func(m_Queue.get());
  return res;
}
VkResult Renderer::Present(VkPresentInfoKHR& info) {
  return m_Queue->Present(info);
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
  return MakeRef<VulkanGraphicsPipeline>(desc, this);
}

void Renderer::AllocateTransientCmdPool(CommandPoolWrapper& pool, VulkanCommandBuffer& cmd,
                                        const char* debugName) {
  pool = m_TransientCmdPoolManager->AllocatePool(debugName);
  VkCommandBufferAllocateInfo buffAllocInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  buffAllocInfo.commandPool = pool;
  buffAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  buffAllocInfo.commandBufferCount = 1;

  auto vkCmdBuff = m_LogicalDevice->AllocateCmdBuffer(buffAllocInfo);

  VkCommandBufferBeginInfo cmdBuffBeginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  cmdBuffBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  auto err = vkBeginCommandBuffer(vkCmdBuff, &cmdBuffBeginInfo);
  FOO_ASSERT(err == VK_SUCCESS);
  cmd.SetVkCommandBuffer(vkCmdBuff, 0, 0);
}

void Renderer::ExecuteAndDisposeTransientCmdBuff(VkCommandBuffer cmd, CommandPoolWrapper&& pool) {
  auto err = vkEndCommandBuffer(cmd);
  FOO_ASSERT(err == VK_SUCCESS);

  VkSubmitInfo submitInfo {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd;
  m_Queue->Submit(submitInfo);
  m_Queue->WaitIdle();
  m_TransientCmdPoolManager->DestroyPools();

  m_LogicalDevice->FreeCmdBuffer(pool, cmd);
  m_TransientCmdPoolManager->Recycle(std::move(pool));
}

void Renderer::Destroy() {
  m_TransientCmdPoolManager->DestroyPools();
  WaitGPU();
}

}  // namespace fg
