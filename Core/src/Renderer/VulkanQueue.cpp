#include "VulkanQueue.h"
#include "VulkanLogicalDevice.h"
#include "RenderContext.h"

namespace fg {

VulkanQueue::VulkanQueue(std::shared_ptr<VulkanLogicalDevice> logicalDevice,
                         const RenderContextInfo& info, uint32_t queueIndex)
    : m_LogicalDevice(logicalDevice),
      m_Queue(logicalDevice->GetQueue(queueIndex)),
      m_QueueIndex(queueIndex) {
}

}  // namespace fg
