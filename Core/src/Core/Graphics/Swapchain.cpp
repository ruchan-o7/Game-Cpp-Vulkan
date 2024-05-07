#include "Swapchain.h"
#include <vulkan/vulkan.h>
#include "../Backend/VulkanCheckResult.h"
#include "../Core/Base.h"
#include "Core/Graphics/Image.h"
#include "vulkan/vulkan_core.h"
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
    Swapchain::Swapchain(SwapchainCreateInfo info) : m_Info(info)
    {
        Init();
    }
    void Swapchain::Destroy()
    {
        for (auto fb : m_SwapchainFrameBuffers)
        {
            vkDestroyFramebuffer(m_Info.device->GetDevice(), fb, nullptr);
        }
        for (auto iV : m_SwapchainImageViews)
        {
            vkDestroyImageView(m_Info.device->GetDevice(), iV, nullptr);
        }
        DestroyImage(m_DepthImage);
        vkDestroySwapchainKHR(m_Info.device->GetDevice(), m_Swapchain, nullptr);
    }
    Swapchain::~Swapchain()
    {
        Destroy();
    }
    void Swapchain::Recreate(VkExtent2D extent)
    {
        Destroy();
        m_Info.extent = extent;
        Init();
        CreateFramebuffers();
    }
    void Swapchain::Init()
    {
        m_Extent      = m_Info.extent;
        m_PresentMode = m_Info.presentMode;
        VkSwapchainCreateInfoKHR swapchainCreate{};
        auto caps               = m_Info.device->GetSurfaceCaps(m_Info.surface);
        swapchainCreate.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreate.surface = m_Info.surface;

        swapchainCreate.minImageCount = caps.minImageCount;
        swapchainCreate.imageFormat =
            chooseSwapSurfaceFormat(
                m_Info.device->GetSurfaceFormats(m_Info.surface))
                .format;
        m_ImageFormat                    = swapchainCreate.imageFormat;
        swapchainCreate.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainCreate.imageExtent      = m_Info.extent;
        swapchainCreate.imageArrayLayers = 1;
        swapchainCreate.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        swapchainCreate.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreate.preTransform     = caps.currentTransform;
        swapchainCreate.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreate.presentMode      = m_Info.presentMode;
        swapchainCreate.clipped          = VK_TRUE;
        swapchainCreate.oldSwapchain     = m_Info.oldSwapchain;

        VK_CALL(vkCreateSwapchainKHR(m_Info.device->GetDevice(),
                                     &swapchainCreate, nullptr, &m_Swapchain));

        VK_CALL(vkGetSwapchainImagesKHR(m_Info.device->GetDevice(), m_Swapchain,
                                        &m_ImageCount, nullptr));
        m_SwapchainImages.resize(m_ImageCount);
        VK_CALL(vkGetSwapchainImagesKHR(m_Info.device->GetDevice(), m_Swapchain,
                                        &m_ImageCount,
                                        m_SwapchainImages.data()));
        {
            m_SwapchainImageViews.resize(m_SwapchainImages.size());
            for (u32 i = 0; i < m_SwapchainImages.size(); i++)
            {
                CreateImageView(m_SwapchainImages[i], m_SwapchainImageViews[i],
                                m_ImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
            }
        }
        CreateDepthResources();
    }
    void Swapchain::CreateDepthResources()
    {
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        CreateImage(m_DepthImage, m_Info.extent, depthFormat,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    void Swapchain::CreateFramebuffers()
    {
        u32 ivSize = m_SwapchainImageViews.size();
        m_SwapchainFrameBuffers.resize(ivSize);

        for (size_t i = 0; i < ivSize; i++)
        {
            VkImageView attachments[] = {GetImageView(i),
                                         m_DepthImage.ImageView};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = *m_Info.renderPass;
            framebufferInfo.attachmentCount = ARRAY_COUNT(attachments);
            framebufferInfo.pAttachments    = attachments;
            framebufferInfo.width           = GetExtent().width;
            framebufferInfo.height          = GetExtent().height;
            framebufferInfo.layers          = 1;

            VK_CALL(vkCreateFramebuffer(m_Info.device->GetDevice(),
                                        &framebufferInfo, nullptr,
                                        &m_SwapchainFrameBuffers[i]));
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
        m_Info.renderPass = renderPass;
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
