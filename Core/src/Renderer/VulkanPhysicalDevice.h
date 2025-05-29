#pragma once

#include "vulkan/vulkan_core.h"
namespace fg {

class VulkanPhysicalDevice {
  public:
    VulkanPhysicalDevice(VkPhysicalDevice device);

    VkPhysicalDevice GetHandle() const {
      return m_Device;
    }
    int GetQueuFamilyIndices(VkQueueFlagBits flags);

  private:
    VkPhysicalDevice m_Device;
};
}  // namespace fg
