#include "RenderDevice.h"
#include <cassert>
#include "VulkanRenderpass.h"
#include "VulkanSwapchain.h"
#include "VulkanBuffer.h"
#include "../Engine/VulkanCheckResult.h"
#include "vulkan/vulkan_core.h"
namespace ENGINE_NAMESPACE
{

    RenderDevice::RenderDevice(EngineFactory* engineFactory, const EngineCreateInfo& engineCI,
                               const GraphicsAdapterInfo& adapterInfo,
                               std::shared_ptr<VulkanInstance> VulkanInstance,
                               std::shared_ptr<VulkanLogicalDevice> LogicalDevice,
                               std::unique_ptr<VulkanPhysicalDevice> PhysicalDevice)
        : m_VulkanInstance(VulkanInstance),
          m_LogicalDevice(LogicalDevice),
          m_PhysicalDevice(std::move(PhysicalDevice))

    {
    }
    RenderDevice::~RenderDevice()
    {
        IdleGPU();
    }

    void RenderDevice::WaitIdle()
    {
        m_LogicalDevice->WaitIdle();
    }

    void RenderDevice::IdleGPU()
    {
        m_LogicalDevice->WaitIdle();
    }

    void RenderDevice::CreateRenderPass(const RenderPassDesc& desc, VulkanRenderPass** pRenderPass)
    {
        *pRenderPass = new VulkanRenderPass(this, desc);
    }

    void RenderDevice::CreateFramebuffer(VulkanSwapchain* pSwapchain, VkFramebuffer* pFramebuffer,
                                         VulkanRenderPass* pRenderPass, uint32_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            VkImageView attachments[] = {pSwapchain->GetImageView(i),
                                         pSwapchain->GetDepthImageView()};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = pRenderPass->GetRenderPass();

            framebufferInfo.attachmentCount = ARRAY_COUNT(attachments);
            framebufferInfo.pAttachments    = attachments;
            framebufferInfo.width           = pSwapchain->GetExtent().width;
            framebufferInfo.height          = pSwapchain->GetExtent().height;
            framebufferInfo.layers          = 1;

            VK_CALL(vkCreateFramebuffer(m_LogicalDevice->GetVkDevice(), &framebufferInfo, nullptr,
                                        &pFramebuffer[i]));
        }
    }
    void RenderDevice::CopyBuffer(VulkanBuffer& source, VulkanBuffer& destination, size_t size)
    {
        // auto device = m_LogicalDevice->GetVkDevice();
        // VkCommandBufferAllocateInfo allocInfo{};
        // allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        // allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        // allocInfo.commandPool        = m_CommandPool;
        // allocInfo.commandBufferCount = 1;
        //
        // VkCommandBuffer commandBuffer;
        // vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
        //
        // VkCommandBufferBeginInfo beginInfo{};
        // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        //
        // vkBeginCommandBuffer(commandBuffer, &beginInfo);
        //
        // VkBufferCopy copyRegion{};
        // copyRegion.size = size;
        // vkCmdCopyBuffer(commandBuffer, source.GetBuffer(), destination.GetBuffer(), 1,
        // &copyRegion);
        //
        // vkEndCommandBuffer(commandBuffer);
        //
        // VkSubmitInfo submitInfo{};
        // submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        // submitInfo.commandBufferCount = 1;
        // submitInfo.pCommandBuffers    = &commandBuffer;
        //
        // auto queue = m_LogicalDevice->GetQueue(VK_QUEUE_GRAPHICS_BIT, 0);
        // vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        // vkQueueWaitIdle(queue);
        //
        // vkFreeCommandBuffers(device, m_CommandPool, 1, &commandBuffer);
    }
}  // namespace ENGINE_NAMESPACE
