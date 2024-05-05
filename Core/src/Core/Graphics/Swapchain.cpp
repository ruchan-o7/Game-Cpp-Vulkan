#include "Swapchain.h"
#include <vulkan/vulkan_core.h>
#include "../Backend/VulkanCheckResult.h"
#include "GLFW/glfw3.h"
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
    void Swapchain::Init(SwapchainCreateInfo swapchainCreateInfo)
    {
        VkSwapchainCreateInfoKHR swapchainCreate{};
        auto caps = swapchainCreateInfo.device->GetSurfaceCaps(
            *swapchainCreateInfo.surface);
        swapchainCreate.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreate.surface = *swapchainCreateInfo.surface;

        swapchainCreate.minImageCount = caps.minImageCount + 1;
        swapchainCreate.imageFormat =
            chooseSwapSurfaceFormat(
                swapchainCreateInfo.device->GetSurfaceFormats(
                    *swapchainCreateInfo.surface))
                .format;
        swapchainCreate.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainCreate.imageExtent      = swapchainCreateInfo.extent;
        swapchainCreate.imageArrayLayers = 1;
        swapchainCreate.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        swapchainCreate.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreate.preTransform     = caps.currentTransform;
        swapchainCreate.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreate.presentMode      = swapchainCreateInfo.presentMode;
        swapchainCreate.clipped          = VK_TRUE;
        swapchainCreate.oldSwapchain     = swapchainCreateInfo.oldSwapchain;

        VK_CALL(vkCreateSwapchainKHR(swapchainCreateInfo.device->GetDevice(),
                                     &swapchainCreate, nullptr, &m_Swapchain));

        VK_CALL(vkGetSwapchainImagesKHR(swapchainCreateInfo.device->GetDevice(),
                                        m_Swapchain, &m_ImageCount, nullptr));
        m_SwapchainImages.resize(m_ImageCount);
        VK_CALL(vkGetSwapchainImagesKHR(swapchainCreateInfo.device->GetDevice(),
                                        m_Swapchain, &m_ImageCount,
                                        m_SwapchainImages.data()));
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

            VK_CALL(vkCreateImageView(swapchainCreateInfo.device->GetDevice(),
                                      &createInfo, nullptr,
                                      &m_SwapchainImagesViews[i]));
        }
    }

    SwapchainBuilder::SwapchainBuilder(Device* device, VkSurfaceKHR* surface)
    {
        createInfo.device      = device;
        createInfo.surface     = surface;
        createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }

    SwapchainBuilder& SwapchainBuilder::SetOldSwapchain(
        VkSwapchainKHR oldSwapchain)
    {
        createInfo.oldSwapchain = oldSwapchain;
        return *this;
    }

    Swapchain SwapchainBuilder::Build()
    {
        i32 w, h;
        glfwGetFramebufferSize(createInfo.window, &w, &h);
        createInfo.extent = {static_cast<uint32_t>(w),
                             static_cast<uint32_t>(h)};
        Swapchain sc;
        sc.Init(createInfo);
        return sc;
    }
}  // namespace FooGame
