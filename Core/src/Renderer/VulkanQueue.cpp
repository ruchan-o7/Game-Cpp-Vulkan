#include "VulkanQueue.h"
#include "VulkanLogicalDevice.h"
#include "RenderContext.h"

namespace fg {

VulkanQueue::VulkanQueue(ReferenceCounter* counter,
                         std::shared_ptr<VulkanLogicalDevice> logicalDevice,
                         const RenderContextInfo& info, uint32_t queueIndex)
    : RefBase(counter),
      m_LogicalDevice(logicalDevice),
      m_Queue(logicalDevice->GetQueue(queueIndex)),
      m_QueueIndex(queueIndex) {
}

VulkanQueue::~VulkanQueue() {
}

VkResult VulkanQueue::Submit(const VkSubmitInfo& submitInfo, uint32_t count, VkFence fence) {
  FOO_ASSERT(submitInfo.sType == VK_STRUCTURE_TYPE_SUBMIT_INFO);
  return vkQueueSubmit(m_Queue, count, &submitInfo, fence);
}

VkResult VulkanQueue::Present(const VkPresentInfoKHR& info) {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_PRESENT_INFO_KHR);
  return vkQueuePresentKHR(m_Queue, &info);
}

void VulkanQueue::WaitIdle() {
  vkQueueWaitIdle(m_Queue);
}

}  // namespace fg
