#include "VulkanLogicalDevice.h"
#include "vulkan/vulkan_core.h"
#include "VulkanObject.h"

namespace fg {

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
}  // namespace fg
