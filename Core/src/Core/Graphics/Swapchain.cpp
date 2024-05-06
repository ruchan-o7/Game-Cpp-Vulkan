#include "Swapchain.h"
#include <vulkan/vulkan.h>
#include "../Backend/VulkanCheckResult.h"
#include "../Core/Base.h"
namespace FooGame
{
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }
    Swapchain::Swapchain(SwapchainCreateInfo info) : m_SwapchainCreateInfo(info)
    {
        Init();
    }
    void Swapchain::Destroy()
    {
        vkDeviceWaitIdle(m_SwapchainCreateInfo.device->GetDevice());
        for (auto fb : m_SwapchainFrameBuffers)
        {
            vkDestroyFramebuffer(m_SwapchainCreateInfo.device->GetDevice(), fb,
                                 nullptr);
        }
        for (auto iV : m_SwapchainImagesViews)
        {
            vkDestroyImageView(m_SwapchainCreateInfo.device->GetDevice(), iV,
                               nullptr);
        }
        vkDestroySwapchainKHR(m_SwapchainCreateInfo.device->GetDevice(),
                              m_Swapchain, nullptr);
    }
    Swapchain::~Swapchain()
    {
        Destroy();
    }
    void Swapchain::Recreate(VkExtent2D extent)
    {
        Destroy();
        m_SwapchainCreateInfo.extent = extent;
        Init();
        CreateFramebuffers();
    }
    void Swapchain::Init()
    {
        m_Extent      = m_SwapchainCreateInfo.extent;
        m_PresentMode = m_SwapchainCreateInfo.presentMode;
        VkSwapchainCreateInfoKHR swapchainCreate{};
        auto caps = m_SwapchainCreateInfo.device->GetSurfaceCaps(
            m_SwapchainCreateInfo.surface);
        swapchainCreate.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreate.surface = m_SwapchainCreateInfo.surface;

        swapchainCreate.minImageCount = caps.minImageCount + 1;
        swapchainCreate.imageFormat =
            chooseSwapSurfaceFormat(
                m_SwapchainCreateInfo.device->GetSurfaceFormats(
                    m_SwapchainCreateInfo.surface))
                .format;
        m_ImageFormat                    = swapchainCreate.imageFormat;
        swapchainCreate.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainCreate.imageExtent      = m_SwapchainCreateInfo.extent;
        swapchainCreate.imageArrayLayers = 1;
        swapchainCreate.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        swapchainCreate.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreate.preTransform     = caps.currentTransform;
        swapchainCreate.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreate.presentMode      = m_SwapchainCreateInfo.presentMode;
        swapchainCreate.clipped          = VK_TRUE;
        swapchainCreate.oldSwapchain     = m_SwapchainCreateInfo.oldSwapchain;

        VK_CALL(vkCreateSwapchainKHR(m_SwapchainCreateInfo.device->GetDevice(),
                                     &swapchainCreate, nullptr, &m_Swapchain));

        VK_CALL(
            vkGetSwapchainImagesKHR(m_SwapchainCreateInfo.device->GetDevice(),
                                    m_Swapchain, &m_ImageCount, nullptr));
        m_SwapchainImages.resize(m_ImageCount);
        VK_CALL(vkGetSwapchainImagesKHR(
            m_SwapchainCreateInfo.device->GetDevice(), m_Swapchain,
            &m_ImageCount, m_SwapchainImages.data()));
        m_SwapchainImagesViews.resize(m_SwapchainImages.size());
        for (size_t i = 0; i < m_SwapchainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image        = m_SwapchainImages[i];
            createInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format       = swapchainCreate.imageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel   = 0;
            createInfo.subresourceRange.levelCount     = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount     = 1;

            VK_CALL(vkCreateImageView(m_SwapchainCreateInfo.device->GetDevice(),
                                      &createInfo, nullptr,
                                      &m_SwapchainImagesViews[i]));
        }
    }
    void Swapchain::CreateFramebuffers()
    {
        u32 ivSize = m_SwapchainImagesViews.size();
        m_SwapchainFrameBuffers.resize(ivSize);

        for (size_t i = 0; i < ivSize; i++)
        {
            VkImageView attachments[] = {GetImageView(i)};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = *m_SwapchainCreateInfo.renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments    = attachments;
            framebufferInfo.width           = GetExtent().width;
            framebufferInfo.height          = GetExtent().height;
            framebufferInfo.layers          = 1;

            VK_CALL(vkCreateFramebuffer(
                m_SwapchainCreateInfo.device->GetDevice(), &framebufferInfo,
                nullptr, &m_SwapchainFrameBuffers[i]));
        }
    }
    VkFramebuffer Swapchain::GetFrameBuffer(u32 imageIndex)
    {
        return m_SwapchainFrameBuffers[imageIndex];
    }
    VkResult Swapchain::AcquireNextImage(VkDevice device, Semaphore& semaphore,
                                         u32* imageIndex)
    {
        return vkAcquireNextImageKHR(device, m_Swapchain, UINT64_MAX,
                                     semaphore.Get(), VK_NULL_HANDLE,
                                     imageIndex);
    }

    SwapchainBuilder::SwapchainBuilder(Device& device, VkSurfaceKHR& surface)
    {
        createInfo.device      = &device;
        createInfo.surface     = surface;
        createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }

    SwapchainBuilder& SwapchainBuilder::SetOldSwapchain(
        VkSwapchainKHR oldSwapchain)
    {
        createInfo.oldSwapchain = oldSwapchain;
        return *this;
    }
    SwapchainBuilder& SwapchainBuilder::SetExtent(VkExtent2D extent)
    {
        createInfo.extent = extent;
        return *this;
    }
    void Swapchain::SetRenderpass(VkRenderPass* renderPass)
    {
        m_SwapchainCreateInfo.renderPass = renderPass;
    }
    SwapchainBuilder& SwapchainBuilder::SetSurface(VkSurfaceKHR surface)
    {
        createInfo.surface = surface;
        return *this;
    }
    SwapchainBuilder& SwapchainBuilder::SetPresentMode(
        VkPresentModeKHR presentMode)
    {
        createInfo.presentMode = presentMode;
        return *this;
    }

    Shared<Swapchain> SwapchainBuilder::Build()
    {
        return CreateShared<Swapchain>(createInfo);
    }
}  // namespace FooGame
