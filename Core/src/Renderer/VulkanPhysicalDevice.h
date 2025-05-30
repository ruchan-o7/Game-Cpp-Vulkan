#pragma once

#include "Volk/volk.h"
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
