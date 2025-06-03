#pragma once
#include <deque>
#include "VulkanObject.h"

namespace fg {

class VulkanCommandBufferPool {
  public:
    VulkanCommandBufferPool(std::shared_ptr<const VulkanLogicalDevice> logical,
                            uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);
    ~VulkanCommandBufferPool();

    VulkanCommandBufferPool(VulkanCommandBufferPool&&) = delete;
    VulkanCommandBufferPool(const VulkanCommandBufferPool&) = delete;
    VulkanCommandBufferPool& operator=(VulkanCommandBufferPool&&) = delete;
    VulkanCommandBufferPool& operator=(const VulkanCommandBufferPool&) = delete;

    VkCommandBuffer Get(const char* debugName = nullptr);
    void Recycle(VkCommandBuffer&&);

  private:
    std::shared_ptr<const VulkanLogicalDevice> m_Device;
    CommandPoolWrapper m_CmdPool;
    std::deque<VkCommandBuffer> m_CmdBuffers;
    const VkPipelineStageFlags m_SupportedStagesMask;
    const VkAccessFlags m_SupportedAccessMask;
};

}  // namespace fg
