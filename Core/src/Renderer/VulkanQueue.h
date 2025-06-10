#pragma once
#include "../Core/Ref.h"
#include "VulkanHeader.h"

namespace fg {
struct RenderContextInfo;
class VulkanLogicalDevice;

class VulkanQueue : public RefBase {
  public:
    VulkanQueue(std::shared_ptr<VulkanLogicalDevice> logicalDevice, const RenderContextInfo& info,
                uint32_t queueIndex);
    uint32_t GetFamilyIndex() const {
      return m_QueueIndex;
    }
    virtual ~VulkanQueue();

  private:
    std::shared_ptr<VulkanLogicalDevice> m_LogicalDevice;
    VkQueue m_Queue = VK_NULL_HANDLE;
    uint32_t m_QueueIndex = 0;
};

}  // namespace fg
