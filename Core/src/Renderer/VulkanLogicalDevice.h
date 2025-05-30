#pragma once

#include <memory>
#include "vulkan/vulkan_core.h"

namespace fg {

template <class T>
class VulkanObject;

using ShaderModuleWrapper = VulkanObject<VkShaderModule>;

class VulkanLogicalDevice : public std::enable_shared_from_this<VulkanLogicalDevice> {
  public:
    VulkanLogicalDevice(VkDevice device, uint32_t queueIndex,
                        const VkAllocationCallbacks* allocator)
        : m_Device(device), m_Allocator(allocator) {
      vkGetDeviceQueue(m_Device, queueIndex, 0, &m_Queue);
    }

    VulkanLogicalDevice(const VulkanLogicalDevice&) = delete;
    VulkanLogicalDevice(VulkanLogicalDevice&&) = delete;
    VulkanLogicalDevice& operator=(const VulkanLogicalDevice&) = delete;
    VulkanLogicalDevice& operator=(VulkanLogicalDevice&&) = delete;

    std::shared_ptr<VulkanLogicalDevice> GetPtr() {
      return shared_from_this();
    }

    std::shared_ptr<const VulkanLogicalDevice> GetPtr() const {
      return shared_from_this();
    }

    VkDevice GetHandle() const {
      return m_Device;
    }
    const VkAllocationCallbacks* GetAllocator() const {
      return m_Allocator;
    }
    void DestroyObject(ShaderModuleWrapper&& module) const;

    ShaderModuleWrapper CreateShader(const VkShaderModuleCreateInfo& info,
                                     const char* name = nullptr);

  private:
    const VkAllocationCallbacks* m_Allocator;
    VkDevice m_Device;
    VkQueue m_Queue;
    uint32_t m_QueueIndex;
};
}  // namespace fg
