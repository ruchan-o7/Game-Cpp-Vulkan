#include "Semaphore.h"
#include <vulkan/vulkan_core.h>
#include "Core/Backend/VulkanCheckResult.h"

namespace FooGame
{
    Fence::Fence(VkDevice* device) : m_Device(device)
    {
        VkFenceCreateInfo c{};
        c.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        c.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CALL(vkCreateFence(*device, &c, nullptr, &m_Fence));
    }
    Semaphore::Semaphore(VkDevice* device) : m_Device(device)
    {
        VkSemaphoreCreateInfo c{};
        c.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CALL(vkCreateSemaphore(*device, &c, nullptr, &m_Semaphore));
    }
}  // namespace FooGame
