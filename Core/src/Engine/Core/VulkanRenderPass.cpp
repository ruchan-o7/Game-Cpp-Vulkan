#include "VulkanRenderpass.h"
#include "RenderDevice.h"
#include <cassert>
#include "TypeConversion.h"
#include "VulkanLogicalDevice.h"
namespace ENGINE_NAMESPACE
{

    VulkanRenderPass::VulkanRenderPass(RenderDevice* pRenderDevice, const RenderPassDesc& desc)
        : m_RenderPass(VK_NULL_HANDLE), m_pRenderDevice(pRenderDevice), m_Desc(desc)
    {
        std::vector<VkAttachmentDescription> attachments(m_Desc.AttachmentCount);
        for (size_t i = 0; i < m_Desc.AttachmentCount; i++)
        {
            const auto& att = m_Desc.pAttachments[i];
            VkAttachmentDescription attachment{};
            attachment.format  = TexFormatToVkFormat(att.Format);
            attachment.samples = static_cast<VkSampleCountFlagBits>(att.SampleCount);

            attachment.loadOp  = LoadOpToVk(att.LoadOp);
            attachment.storeOp = StoreOpToVk(att.StoreOp);

            attachment.stencilLoadOp  = LoadOpToVk(att.StencilLoadOp);
            attachment.stencilStoreOp = StoreOpToVk(att.StencilStoreOp);

            attachment.initialLayout = RsToImageLayout(att.InitialLayout);

            attachment.finalLayout = RsToImageLayout(att.FinalLayout);
            attachments[i]         = attachment;
        }

        std::vector<VkAttachmentReference> refs{m_Desc.pSubpassDesc->AttachmentCount};
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass   = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass   = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;
        m_RenderPass = m_pRenderDevice->GetLogicalDevice()->CreateRenderPass(renderPassInfo);
    }
}  // namespace ENGINE_NAMESPACE
