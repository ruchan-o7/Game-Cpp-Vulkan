#include "Semaphore.h"
#include <vulkan/vulkan_core.h>
#include "../Backend/VulkanCheckResult.h"

namespace FooGame
{
    Fence::Fence(VkDevice device) : m_Device(device)
    {
        VkFenceCreateInfo c{};
        c.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        c.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CALL(vkCreateFence(m_Device, &c, nullptr, &m_Fence));
    }
    void Fence::Wait(u64 timeOut)
    {
        vkWaitForFences(m_Device, 1, &m_Fence, VK_TRUE, timeOut);
    }
    void Fence::Reset()
    {
        vkResetFences(m_Device, 1, &m_Fence);
    }
    void Fence::Destroy(VkDevice device)
    {
        vkDestroyFence(device, m_Fence, nullptr);
    }
    Semaphore::Semaphore(VkDevice device) : m_Device(device)
    {
        VkSemaphoreCreateInfo c{};
        c.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CALL(vkCreateSemaphore(m_Device, &c, nullptr, &m_Semaphore));
    }
    void Semaphore::Destroy(VkDevice device)
    {
        vkDestroySemaphore(device, m_Semaphore, nullptr);
    }
}  // namespace FooGame
