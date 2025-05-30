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

void VulkanLogicalDevice::DestroyObject(ShaderModuleWrapper&& module) const {
  vkDestroyShaderModule(m_Device, module, m_Allocator);
  module.m_VulkanObject = VK_NULL_HANDLE;
}

}  // namespace fg
