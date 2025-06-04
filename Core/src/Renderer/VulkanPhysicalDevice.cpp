#include "VulkanPhysicalDevice.h"

namespace fg {

VulkanPhysicalDevice ::VulkanPhysicalDevice(VkPhysicalDevice device) : m_Device(device) {
  vkGetPhysicalDeviceProperties(m_Device, &m_Properties);
  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &count, nullptr);
  m_QueueFamilyProps.resize(count);
  vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &count, m_QueueFamilyProps.data());
}

int VulkanPhysicalDevice::GetQueuFamilyIndices(VkQueueFlagBits flags) {
  flags = VK_QUEUE_GRAPHICS_BIT;
  int idx = 0;
  for (const auto& prop : m_QueueFamilyProps) {
    if (prop.queueFlags & flags) {
      return idx;
    }
    idx++;
  }
  return 0;
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
