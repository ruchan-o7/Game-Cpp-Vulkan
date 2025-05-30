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

}  // namespace fg
