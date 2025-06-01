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
    uint32_t FindMemTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags flags) const;

  private:
    VkPhysicalDevice m_Device;
};
}  // namespace fg
