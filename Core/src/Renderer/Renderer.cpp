#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include "Renderer.h"
#include "CommandPoolManager.h"
#include "VulkanCommandBufferPool.h"
#include "VulkanDebug.h"
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

// const char* validationLayers[] = {
//     "VK_LAYER_KHRONOS_validation",
// };

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
// std::shared_ptr<Renderer> Renderer::Create(GLFWwindow* window, const VkAllocationCallbacks* acb)
// {
//   if (volkInitialize() != VK_SUCCESS) {
//     return nullptr;
//   }
//   VkInstance vkInstance = VK_NULL_HANDLE;
//
//   VkApplicationInfo appInfo {VK_STRUCTURE_TYPE_APPLICATION_INFO};
//   appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
//   appInfo.pApplicationName = "Foo game engine";
//   appInfo.pEngineName = "FG Engine";
//   appInfo.apiVersion = VK_API_VERSION_1_3;
//
//   VkInstanceCreateInfo pCreateInfo {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
//   pCreateInfo.pApplicationInfo = &appInfo;
//
//   uint32_t extCount = 0;
//   const char** glfwExtensions = nullptr;
//   glfwExtensions = glfwGetRequiredInstanceExtensions(&extCount);
//   std::vector<const char*> instanceExtensions;
//   instanceExtensions.reserve(extCount + 1);
//   for (int i = 0; i < extCount; i++) {
//     instanceExtensions.emplace_back(glfwExtensions[i]);
//   }
//   instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//
//   pCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
//   pCreateInfo.enabledExtensionCount = instanceExtensions.size();
//   pCreateInfo.ppEnabledLayerNames = validationLayers;
//   pCreateInfo.enabledLayerCount = 1;
//
//   VkResult res = vkCreateInstance(&pCreateInfo, acb, &vkInstance);
//
//   volkLoadInstance(vkInstance);
//
//   if (res != VK_SUCCESS) {
//     throw std::runtime_error("Can not create vulkan instance");
//   }
//
//   auto Instance = std::make_shared<VulkanInstance>(vkInstance, acb);
//
//   auto messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
//                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
//                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//   auto messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
//                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
//                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//   SetupDebugUtils(vkInstance, messageSeverity, messageType, 0, nullptr);
//
//   VkPhysicalDevice physicalDevices[8];
//   uint32_t physicalDeviceCount = 0;
//   vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
//   vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices);
//   if (physicalDeviceCount == 0) {
//     throw std::runtime_error("Can not supported physical GPU device");
//   }
//   VkPhysicalDevice selected = VK_NULL_HANDLE;
//   for (uint32_t i = 0; i < physicalDeviceCount; i++) {
//     VkPhysicalDeviceProperties props;
//     vkGetPhysicalDeviceProperties(physicalDevices[i], &props);
//     if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
//       selected = physicalDevices[i];
//       break;
//     }
//   }
//   if (selected == VK_NULL_HANDLE) {
//     FOO_CORE_INFO("Can not find discrete GPU, selecting first one");
//     selected = physicalDevices[0];
//   }
//   VkPhysicalDeviceProperties pDeviceProps {};
//   vkGetPhysicalDeviceProperties(selected, &pDeviceProps);
//   FOO_CORE_INFO("Selected GPU: {}", pDeviceProps.deviceName);
//
//   auto physicalDevice = std::make_unique<VulkanPhysicalDevice>(selected);
//
//   Renderer* renderer = new Renderer(window, Instance, std::move(physicalDevice), acb);
//
//   return std::shared_ptr<Renderer>(renderer);
// }

void Renderer::WaitGPU() const {
  m_LogicalDevice->WaitIdle();
}

// Renderer::Renderer(GLFWwindow* window, const std::shared_ptr<VulkanInstance>& instance,
//                    std::unique_ptr<VulkanPhysicalDevice> pDevice,
//                    const VkAllocationCallbacks* alloc)
//     : m_Instance(instance),
//       m_Window(window),
//       m_PhysicalDevice(std::move(pDevice)),
//       m_AllocCB(alloc) {
// }

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

  // VkDeviceCreateInfo info {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  // info.pQueueCreateInfos = &queueInfo;
  // info.queueCreateInfoCount = 1;
  // info.pEnabledFeatures = &features;
  // info.enabledExtensionCount = 2;
  // info.ppEnabledExtensionNames = extensions;
  // info.enabledLayerCount = 1;
  // info.ppEnabledLayerNames = validationLayers;
  // info.pNext = &dynamicRendering;

  // m_LogicalDevice =
  //     std::make_shared<VulkanLogicalDevice>(info, queueIndex, nullptr /*TODO:*/, m_AllocCB);
  // m_VkQueue = m_LogicalDevice->GetQueue(0);

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

// void Renderer::BindDescriptorSet(VulkanDescriptorSet* set) {
//   FOO_ASSERT(m_CurrentPipeline != nullptr);
//   auto vkSet = set->GetHandle();
//   m_Cmd.BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, m_CurrentPipeline->Layout(), 0, 1,
//                            &vkSet, 0, nullptr);
// }

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

// void Renderer::SetRenderTargetToSwapchain() {
//   auto cmd = m_CmdPool->Get();
//   m_Cmd.SetVkCommandBuffer(cmd, 0, 0);
//
//   TransitionImageLayout(m_Swapchain->GetCurrentImageView()->GetImage(),
//                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
//
//   VkRenderingInfoKHR beginInfo {VK_STRUCTURE_TYPE_RENDERING_INFO, 0};
//   beginInfo.renderArea = {
//       {0, 0},
//       m_Swapchain->GetExtent()
//   };
//   beginInfo.layerCount = 1;
//
//   VkRenderingAttachmentInfo colorInfo {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR};
//   colorInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//   colorInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//   colorInfo.imageView = m_Swapchain->GetCurrentImageView()->GetHandle();
//   colorInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//   colorInfo.clearValue = {
//       {0.2f, 0.2f, 0.2f, 1.0f}
//   };
//   std::vector<VkRenderingAttachmentInfo> colorAttachments;
//   colorAttachments.push_back(colorInfo);
//
//   beginInfo.colorAttachmentCount = colorAttachments.size();
//   beginInfo.pColorAttachments = colorAttachments.data();
//   m_Cmd.BeginRendering(beginInfo);
//   VkRect2D scissor {
//       {0, 0},
//       m_Swapchain->GetExtent()
//   };
//   m_Cmd.SetScissor(scissor);
//   VkViewport vp {
//       0,    0,   (float)m_Swapchain->GetExtent().width, (float)m_Swapchain->GetExtent().height,
//       0.0f, 1.0f};
//   m_Cmd.CmdSetViewport(0, 1, vp);
// }

// void Renderer::SetRenderTargets(uint32_t count, VulkanImageView* views,
//                                 VulkanImageView* depthView) {
//   memset(m_BoundImages, 0, sizeof(m_BoundImages));
//   m_BoundImageCount = count;
//
//   auto cmd = m_CmdPool->Get();
//   m_Cmd.SetVkCommandBuffer(cmd, 0, 0);
//   VkImageSubresourceRange range {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
//   for (uint32_t i = 0; i < count; i++) {
//     auto* image = views[i].GetImage();
//     m_BoundImages[i] = image;
//     m_Cmd.TransitionImageLayout(
//         image->GetVkImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//         range, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
//   }
//   const auto* image = views[0].GetImage();
//
//   VkRenderingInfoKHR beginInfo {VK_STRUCTURE_TYPE_RENDERING_INFO, 0};
//   beginInfo.renderArea = {
//       {0, 0}
//   };
//   beginInfo.renderArea.extent.width = image->Width();
//   beginInfo.renderArea.extent.height = image->Height();
//   beginInfo.layerCount = 1;
//
//   VkRenderingAttachmentInfo colorInfos[8];
//   memset(colorInfos, 0, sizeof(colorInfos));
//
//   for (uint32_t i = 0; i < count; i++) {
//     auto& colorInfo = colorInfos[i];
//     colorInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
//     colorInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//     colorInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//     colorInfo.imageView = views[i].GetHandle();
//     colorInfo.clearValue = {
//         {0.2f, 0.2f, 0.2f, 1.0f}
//     };
//   }
//   beginInfo.colorAttachmentCount = count;
//   beginInfo.pColorAttachments = colorInfos;
//   VkRenderingAttachmentInfo depthInfo {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR};
//   depthInfo.clearValue = {};
//   depthInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//   depthInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//   if (depthView) {
//     depthInfo.imageView = depthView->GetHandle();
//   }
//   beginInfo.pDepthAttachment = depthView != nullptr ? &depthInfo : nullptr;
//   m_Cmd.BeginRendering(beginInfo);
//   VkRect2D scissor {
//       {             0,               0},
//       {image->Width(), image->Height()},
//   };
//   m_Cmd.SetScissor(scissor);
//   VkViewport vp {0, 0, (float)image->Width(), (float)image->Height(), 0.0f, 1.0f};
//   m_Cmd.CmdSetViewport(0, 1, vp);
// }
//
// void Renderer::EndRendering() {
//   m_Cmd.EndRendering();
//   VkImageSubresourceRange range {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
//   for (uint32_t i = 0; i < m_BoundImageCount; i++) {
//     auto* image = m_BoundImages[i];
//     m_Cmd.TransitionImageLayout(image->GetVkImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range,
//                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//                                 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
//   }
//   m_Cmd.FlushBarriers();
// }
//
// void Renderer::EndRenderingSwapchain() {
//   m_Cmd.EndRendering();
//   TransitionImageLayout(m_Swapchain->GetCurrentImageView()->GetImage(),
//                         VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
//   m_Cmd.FlushBarriers();
// }
//
// void Renderer::BindPipeline(const Ref<VulkanGraphicsPipeline>& pipeline) {
//   m_CurrentPipeline = pipeline;
//   m_Cmd.BindGraphicsPipeline(pipeline->GetHandle());
// }
// void Renderer::Draw(const DrawAttributes& attribs) {
//   // FOO_ASSERT(m_CurrentPipeline != nullptr);
//   // m_Cmd.Draw(attribs.VertexCount, attribs.InstanceCount, attribs.FirstVertex,
//   //            attribs.FirstInstance);
// }

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
  //m_Cmd.EndCommandBuffer();

  auto res = func(m_Queue.get());

  //m_CmdPool->Recycle(m_Cmd.Get());
  //m_Cmd.Reset();

  return res;
}
VkResult Renderer::Present(VkPresentInfoKHR& info) {
  return m_Queue->Present(info);
  // return vkQueuePresentKHR(m_VkQueue, &info);
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

// void Renderer::BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,
//                                  VulkanBuffer** buffers, VkDeviceSize* offsets) const {
//   FOO_ASSERT(bindingCount > 0);
//   FOO_ASSERT(buffers != nullptr);
//   FOO_ASSERT(offsets != nullptr);
//   auto cmd = GetCurrentCmdBuffer();
//   VkBuffer vkbuffers[8];
//   memset(vkbuffers, 0, sizeof(vkbuffers));
//   for (uint32_t i = 0; i < bindingCount; i++) {
//     vkbuffers[i] = buffers[i]->GetVkBuffer();
//   }
//   m_Cmd.BindVertexBuffers(firstBinding, bindingCount, vkbuffers, offsets);
// }

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
