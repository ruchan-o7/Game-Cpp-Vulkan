#pragma once
#include "VulkanLogicalDevice.h"
#include "vulkan/vulkan_core.h"
namespace fg {

VulkanLogicalDevice::VulkanLogicalDevice(VkDevice device, uint32_t queueIndex)
    : m_Device(device), m_QueueIndex(queueIndex) {
  vkGetDeviceQueue(m_Device, queueIndex, 0, &m_Queue);
}

}  // namespace fg
