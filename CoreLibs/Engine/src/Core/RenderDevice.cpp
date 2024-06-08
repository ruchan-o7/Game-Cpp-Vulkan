#include "RenderDevice.h"
#include <cassert>
#include "VulkanRenderpass.h"
#include "VulkanSwapchain.h"
#include "../Engine/VulkanCheckResult.h"
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
}  // namespace ENGINE_NAMESPACE
