#include <stdexcept>
#define NOMINMAX

#include "VulkanPhysicalDevice.h"
#include <limits>

namespace fg {

VulkanPhysicalDevice ::VulkanPhysicalDevice(VkPhysicalDevice device) : m_Device(device) {
  vkGetPhysicalDeviceProperties(m_Device, &m_Properties);
  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &count, nullptr);
  m_QueueFamilyProps.resize(count);
  vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &count, m_QueueFamilyProps.data());
}

int VulkanPhysicalDevice::GetQueuFamilyIndices(VkQueueFlags flags) {
  // flags = VK_QUEUE_GRAPHICS_BIT;
  //
  static constexpr uint32_t invalidQueueFam = std::numeric_limits<uint32_t>::max();

  int idx = invalidQueueFam;
  for (uint32_t i = 0; i < m_QueueFamilyProps.size(); i++) {
    const auto& prop = m_QueueFamilyProps[i];
    if (prop.queueFlags == flags) {
      idx = i;
      break;
    }
  }
  if (idx == invalidQueueFam) {
    for (uint32_t i = 0; i < m_QueueFamilyProps.size(); i++) {
      const auto& prop = m_QueueFamilyProps[i];
      if (prop.queueFlags & flags) {
        idx = i;
        break;
      }
    }
  }
  if (idx == invalidQueueFam) {
    throw std::runtime_error("Can not find suitable queue family indices for flag");
  }
  return idx;
}
uint32_t VulkanPhysicalDevice::FindMemTypeIndex(uint32_t typeBits,
                                                VkMemoryPropertyFlags flags) const {
  VkPhysicalDeviceMemoryProperties memProps;
  vkGetPhysicalDeviceMemoryProperties(m_Device, &memProps);

  for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
    if ((typeBits & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & flags) == flags) {
      return i;
    }
  }
  return -1;
}

}  // namespace fg
