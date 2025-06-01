#include "VulkanPhysicalDevice.h"

namespace fg {

VulkanPhysicalDevice ::VulkanPhysicalDevice(VkPhysicalDevice device) : m_Device(device) {
}

int VulkanPhysicalDevice::GetQueuFamilyIndices(VkQueueFlagBits flags) {
  flags = VK_QUEUE_GRAPHICS_BIT;
  uint32_t count = 0;
  VkQueueFamilyProperties props[30];
  vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &count, nullptr);
  vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &count, props);
  for (int i = 0; i < count; i++) {
    const auto& prop = props[i];
    if (prop.queueFlags & flags) {
      return i;
    }
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
