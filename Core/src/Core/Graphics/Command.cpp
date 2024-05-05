#include "Command.h"
#include <vulkan/vulkan_core.h>
#include "../Backend/VulkanCheckResult.h"
namespace FooGame
{
    Command::Command(const Device& device, u32 commandBufferCount)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = device.GetGraphicsFamily();

        VK_CALL(vkCreateCommandPool(device.GetDevice(), &poolInfo, nullptr,
                                    &m_CommandPool));
        m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

        VK_CALL(vkAllocateCommandBuffers(device.GetDevice(), &allocInfo,
                                         m_CommandBuffers.data()));
    }

    void Command::Reset(u32 index, VkCommandBufferResetFlags flags)
    {
        VK_CALL(vkResetCommandBuffer(m_CommandBuffers[index], flags));
    }
    void Command::StartRecording(u32 index)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CALL(vkBeginCommandBuffer(m_CommandBuffers[index], &beginInfo));
    }
    void Command::StopRecording(u32 index)
    {
        VK_CALL(vkEndCommandBuffer(m_CommandBuffers[index]));
    }
}  // namespace FooGame
