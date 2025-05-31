#include "VulkanLogicalDevice.h"
#include "VulkanObject.h"
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

ShaderModuleWrapper VulkanLogicalDevice::CreateShader(const VkShaderModuleCreateInfo& info,
                                                      const char* name) {
  VkShaderModule module;
  auto res = vkCreateShaderModule(m_Device, &info, m_Allocator, &module);
  return ShaderModuleWrapper(std::move(module), GetPtr());
}
PipelineLayoutWrapper VulkanLogicalDevice::CreatePipelineLayout(
    const VkPipelineLayoutCreateInfo& info, const char* name) {
  VkPipelineLayout layout = VK_NULL_HANDLE;
  auto res = vkCreatePipelineLayout(m_Device, &info, m_Allocator, &layout);
  return PipelineLayoutWrapper(std::move(layout), GetPtr());
}

PipelineWrapper VulkanLogicalDevice::CreateGraphicsPipeline(
    const VkGraphicsPipelineCreateInfo& info, const char* name) {
  VkPipeline handle = VK_NULL_HANDLE;
  auto res = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &info, m_Allocator, &handle);
  return PipelineWrapper(std::move(handle), GetPtr());
}
CommandPoolWrapper VulkanLogicalDevice::CreateCommandPool(const VkCommandPoolCreateInfo& info,
                                                          const char* name) {
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
FenceWrapper VulkanLogicalDevice::CreateFence(const VkFenceCreateInfo& info, const char* name) {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
  VkFence handle;
  auto res = vkCreateFence(m_Device, &info, m_Allocator, &handle);
  return FenceWrapper(std::move(handle), GetPtr());
}

VkCommandBuffer VulkanLogicalDevice::AllocateCmdBuffer(const VkCommandBufferAllocateInfo& info) {
  VkCommandBuffer handle = VK_NULL_HANDLE;
  auto res = vkAllocateCommandBuffers(m_Device, &info, &handle);
  return handle;
}

void VulkanLogicalDevice::DestroyObject(ShaderModuleWrapper&& module) const {
  vkDestroyShaderModule(m_Device, module, m_Allocator);
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
