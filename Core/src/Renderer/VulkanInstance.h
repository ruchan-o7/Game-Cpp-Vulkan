#pragma once

#include "vulkan/vulkan_core.h"
namespace fg {
class VulkanInstance {
  public:
    VulkanInstance(VkInstance instance, const VkAllocationCallbacks* allocator)
        : m_Instance(instance), m_Allocator(allocator) {
    }
    ~VulkanInstance();

    VkInstance GetHandle() const {
      return m_Instance;
    }

    const VkAllocationCallbacks* GetAllocator() const {
      return m_Allocator;
    }

  private:
    VkInstance m_Instance;
    const VkAllocationCallbacks* m_Allocator;
    std::vector<VkLayerProperties> m_AvailableLayers;
    std::vector<VkExtensionProperties> m_AvailableExtensionProperties;
};
}  // namespace fg
