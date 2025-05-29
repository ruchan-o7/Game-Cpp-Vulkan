#pragma once

#include "vulkan/vulkan_core.h"
namespace fg {
class VulkanLogicalDevice {
  public:
    VulkanLogicalDevice(VkDevice device, uint32_t queueIndex);

    VkDevice GetHandle() const {
      return m_Device;
    }

  private:
    VkDevice m_Device;
    VkQueue m_Queue;
    uint32_t m_QueueIndex;
};
}  // namespace fg
