#pragma once

#include "vulkan/vulkan_core.h"
namespace fg {
class VulkanLogicalDevice {
  public:
    VulkanLogicalDevice(VkDevice device, uint32_t queueIndex,
                        const VkAllocationCallbacks* allocator)
        : m_Device(device), m_Allocator(allocator) {
      vkGetDeviceQueue(m_Device, queueIndex, 0, &m_Queue);
    }

    VkDevice GetHandle() const {
      return m_Device;
    }
    const VkAllocationCallbacks* GetAllocator() const {
      return m_Allocator;
    }

    VkShaderModule CreateShader(const VkShaderModuleCreateInfo& info, const char* name = nullptr);

  private:
    const VkAllocationCallbacks* m_Allocator;
    VkDevice m_Device;
    VkQueue m_Queue;
    uint32_t m_QueueIndex;
};
}  // namespace fg
