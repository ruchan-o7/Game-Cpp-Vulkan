#pragma once
#include "VulkanLogicalDevice.h"
#include "vulkan/vulkan_core.h"
namespace fg {

VkShaderModule VulkanLogicalDevice::CreateShader(const VkShaderModuleCreateInfo& info,
                                                 const char* name) {
  VkShaderModule module = VK_NULL_HANDLE;
  auto res = vkCreateShaderModule(GetHandle(), &info, GetAllocator(), &module);
  return module;
}
}  // namespace fg
