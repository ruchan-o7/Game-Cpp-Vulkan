#pragma once

#include "VulkanHeader.h"
namespace fg {

class VulkanPhysicalDevice {
  public:
    VulkanPhysicalDevice(VkPhysicalDevice device);

    VkPhysicalDevice GetHandle() const {
      return m_Device;
    }
    int GetQueuFamilyIndices(VkQueueFlagBits flags);
    uint32_t FindMemTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags flags) const;
    VkPhysicalDeviceProperties GetProperties() const {
      return m_Properties;
    }

  private:
    std::vector<VkQueueFamilyProperties> m_QueueFamilyProps;
    VkPhysicalDeviceProperties m_Properties;
    VkPhysicalDevice m_Device;
};
}  // namespace fg
