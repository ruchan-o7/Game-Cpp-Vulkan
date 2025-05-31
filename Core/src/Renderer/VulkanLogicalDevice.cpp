#include "VulkanLogicalDevice.h"
#include "VulkanObject.h"
#include "VulkanPhysicalDevice.h"
#include "Renderer.h"
#include "../Core/Assert.h"

namespace fg {

void VulkanLogicalDevice::WaitIdle() const {
  vkDeviceWaitIdle(m_Device);
}
void VulkanLogicalDevice::ResetFence(VkFence& fence) {
  vkResetFences(m_Device, 1, &fence);
}

void VulkanLogicalDevice::WaitFence(VkFence fence) {
  vkWaitForFences(m_Device, 1, &fence, VK_TRUE, UINT64_MAX);
}

VkResult VulkanLogicalDevice::GetFenceStatus(VkFence fence) {
  return vkGetFenceStatus(m_Device, fence);
}

VkMemoryRequirements VulkanLogicalDevice::GetImageMemReq(VkImage image) const {
  VkMemoryRequirements memReqs {};
  vkGetImageMemoryRequirements(m_Device, image, &memReqs);
  return memReqs;
}
VkDeviceMemory VulkanLogicalDevice::AllocateMemory(VkMemoryRequirements memReqs) const {
  VkMemoryAllocateInfo info {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  info.allocationSize = memReqs.size;
  info.memoryTypeIndex = m_Renderer.lock()->GetPhysicalDevice().FindMemTypeIndex(
      memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VkDeviceMemory mem = 0;
  vkAllocateMemory(m_Device, &info, m_Allocator, &mem);
  return mem;
}

void VulkanLogicalDevice::BindImageMemory(VkImage image, VkDeviceMemory mem) {
  vkBindImageMemory(m_Device, image, mem, 0);
}

ShaderModuleWrapper VulkanLogicalDevice::CreateShader(const VkShaderModuleCreateInfo& info,
                                                      const char* name) const {
  VkShaderModule module;
  auto res = vkCreateShaderModule(m_Device, &info, m_Allocator, &module);
  return ShaderModuleWrapper(std::move(module), GetPtr());
}
PipelineLayoutWrapper VulkanLogicalDevice::CreatePipelineLayout(
    const VkPipelineLayoutCreateInfo& info, const char* name) const {
  VkPipelineLayout layout = VK_NULL_HANDLE;
  auto res = vkCreatePipelineLayout(m_Device, &info, m_Allocator, &layout);
  return PipelineLayoutWrapper(std::move(layout), GetPtr());
}

PipelineWrapper VulkanLogicalDevice::CreateGraphicsPipeline(
    const VkGraphicsPipelineCreateInfo& info, const char* name) const {
  VkPipeline handle = VK_NULL_HANDLE;
  auto res = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &info, m_Allocator, &handle);
  return PipelineWrapper(std::move(handle), GetPtr());
}
CommandPoolWrapper VulkanLogicalDevice::CreateCommandPool(const VkCommandPoolCreateInfo& info,
                                                          const char* name) const {
  VkCommandPool handle = VK_NULL_HANDLE;
  auto res = vkCreateCommandPool(m_Device, &info, m_Allocator, &handle);
  return CommandPoolWrapper(std::move(handle), GetPtr());
}
SemaphoreWrapper VulkanLogicalDevice::CreateVulkanSemaphore(const VkSemaphoreCreateInfo& info,
                                                            const char* name) {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
  VkSemaphore handle;
  auto res = vkCreateSemaphore(m_Device, &info, m_Allocator, &handle);
  return SemaphoreWrapper(std::move(handle), GetPtr());
}
FenceWrapper VulkanLogicalDevice::CreateFence(const VkFenceCreateInfo& info,
                                              const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
  VkFence handle;
  auto res = vkCreateFence(m_Device, &info, m_Allocator, &handle);
  return FenceWrapper(std::move(handle), GetPtr());
}
ImageWrapper VulkanLogicalDevice::CreateImage(const VkImageCreateInfo& info,
                                              const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
  VkImage handle;
  auto res = vkCreateImage(m_Device, &info, m_Allocator, &handle);
  return ImageWrapper(std::move(handle), GetPtr());
}

VkCommandBuffer VulkanLogicalDevice::AllocateCmdBuffer(
    const VkCommandBufferAllocateInfo& info) const {
  VkCommandBuffer handle = VK_NULL_HANDLE;
  auto res = vkAllocateCommandBuffers(m_Device, &info, &handle);
  return handle;
}

void VulkanLogicalDevice::DestroyObject(ShaderModuleWrapper&& module) const {
  vkDestroyShaderModule(m_Device, module, m_Allocator);
  module.m_VulkanObject = VK_NULL_HANDLE;
}
void VulkanLogicalDevice::DestroyObject(ImageWrapper&& module) const {
  vkDestroyImage(m_Device, module, m_Allocator);
  module.m_VulkanObject = VK_NULL_HANDLE;
}

void VulkanLogicalDevice::DestroyObject(PipelineLayoutWrapper&& handle) const {
  vkDestroyPipelineLayout(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}
void VulkanLogicalDevice::DestroyObject(PipelineWrapper&& handle) const {
  vkDestroyPipeline(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}

void VulkanLogicalDevice::DestroyObject(CommandPoolWrapper&& handle) const {
  vkDestroyCommandPool(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}
void VulkanLogicalDevice::DestroyObject(SemaphoreWrapper&& handle) const {
  vkDestroySemaphore(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}
void VulkanLogicalDevice::DestroyObject(FenceWrapper&& handle) const {
  vkDestroyFence(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}
}  // namespace fg
