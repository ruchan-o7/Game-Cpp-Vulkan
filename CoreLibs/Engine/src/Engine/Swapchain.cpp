#include "Swapchain.h"
#include "Device.h"
#include "VulkanCheckResult.h"
#include "Api.h"
#include "../Defines.h"
#include "../Geometry/AssetLoader.h"
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
    Swapchain::Swapchain(SwapchainCreateInfo info)
        : m_Extent(info.extent), m_PresentMode(info.presentMode), m_Info(info)

    {
        Init();
    }
    void Swapchain::Destroy()
    {
        auto device = Api::GetDevice()->GetDevice();

        AssetLoader::DestroyTexture(m_DepthImage);
        for (auto& fb : m_SwapchainFrameBuffers)
        {
            vkDestroyFramebuffer(device, fb, nullptr);
        }
        for (auto& iV : m_SwapchainImageViews)
        {
            vkDestroyImageView(device, iV, nullptr);
        }
        vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
    }
    Swapchain::~Swapchain()
    {
        Destroy();
    }
    void Swapchain::Recreate(VkExtent2D extent)
    {
        Destroy();
        m_Extent = extent;
        Init();
        CreateFramebuffers();
    }
    static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR caps, const VkExtent2D& ext)
    {
        if (caps.currentExtent.width != UINT32_MAX)
        {
            return caps.currentExtent;
        }
        else
        {
            VkExtent2D actualExtent = ext;
            actualExtent.width      = glm::clamp(actualExtent.width, caps.minImageExtent.width,
                                                 caps.maxImageExtent.width);
            actualExtent.height     = glm::clamp(actualExtent.height, caps.minImageExtent.height,
                                                 caps.maxImageExtent.height);
            return actualExtent;
        }
    }
    void Swapchain::Init()
    {
        auto device   = Api::GetDevice();
        auto vkdevice = device->GetDevice();
        VkSwapchainCreateInfoKHR swapchainCreate{};
        auto caps               = device->GetSurfaceCaps(Api::GetSurface());
        swapchainCreate.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreate.surface = Api::GetSurface();

        swapchainCreate.minImageCount = caps.minImageCount + 1;  // TODO: this can be trouble
        m_ImageFormat =
            chooseSwapSurfaceFormat(device->GetSurfaceFormats(Api::GetSurface())).format;

        swapchainCreate.imageFormat = m_ImageFormat;
        if (caps.maxImageCount > 0 && swapchainCreate.minImageCount > caps.maxImageCount)
        {
            swapchainCreate.minImageCount = caps.maxImageCount;
        }
        swapchainCreate.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainCreate.imageExtent      = ChooseSwapExtent(caps, m_Extent);
        swapchainCreate.imageArrayLayers = 1;
        swapchainCreate.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        swapchainCreate.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreate.preTransform     = caps.currentTransform;
        swapchainCreate.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreate.presentMode      = m_Info.presentMode;
        swapchainCreate.clipped          = VK_TRUE;
        swapchainCreate.oldSwapchain     = m_Info.oldSwapchain;

        VK_CALL(vkCreateSwapchainKHR(vkdevice, &swapchainCreate, nullptr, &m_Swapchain));

        vkGetSwapchainImagesKHR(vkdevice, m_Swapchain, &m_ImageCount, nullptr);

        m_SwapchainImages.resize(m_ImageCount);

        vkGetSwapchainImagesKHR(vkdevice, m_Swapchain, &m_ImageCount, m_SwapchainImages.data());

        m_SwapchainImageViews.resize(m_SwapchainImages.size());
        for (uint32_t i = 0; i < m_SwapchainImages.size(); i++)
        {
            CreateImageView(m_SwapchainImages[i], m_SwapchainImageViews[i], m_ImageFormat,
                            VK_IMAGE_ASPECT_COLOR_BIT);
        }

        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        CreateImage(m_DepthImage, m_Extent, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    void Swapchain::CreateFramebuffers()
    {
        auto device     = Api::GetDevice()->GetDevice();
        uint32_t ivSize = m_SwapchainImageViews.size();
        m_SwapchainFrameBuffers.resize(ivSize);

        for (size_t i = 0; i < ivSize; i++)
        {
            VkImageView attachments[] = {GetImageView(i), m_DepthImage.ImageView};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = m_Info.renderPass;
            framebufferInfo.attachmentCount = ARRAY_COUNT(attachments);
            framebufferInfo.pAttachments    = attachments;
            framebufferInfo.width           = m_Extent.width;
            framebufferInfo.height          = m_Extent.height;
            framebufferInfo.layers          = 1;

            VK_CALL(vkCreateFramebuffer(device, &framebufferInfo, nullptr,
                                        &m_SwapchainFrameBuffers[i]));
        }
    }
    VkFramebuffer Swapchain::GetFrameBuffer(uint32_t imageIndex)
    {
        return m_SwapchainFrameBuffers[imageIndex];
    }
    VkResult Swapchain::AcquireNextImage(VkDevice device, Semaphore& semaphore,
                                         uint32_t* imageIndex)
    {
        return vkAcquireNextImageKHR(device, m_Swapchain, UINT64_MAX, semaphore.Get(),
                                     VK_NULL_HANDLE, imageIndex);
    }

    SwapchainBuilder::SwapchainBuilder()
    {
        createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }

    SwapchainBuilder& SwapchainBuilder::SetOldSwapchain(VkSwapchainKHR oldSwapchain)
    {
        createInfo.oldSwapchain = oldSwapchain;
        return *this;
    }
    SwapchainBuilder& SwapchainBuilder::SetExtent(VkExtent2D extent)
    {
        createInfo.extent = extent;
        return *this;
    }
    void Swapchain::SetRenderpass()
    {
        m_Info.renderPass = Api::GetRenderpass();
    }
    SwapchainBuilder& SwapchainBuilder::SetPresentMode(VkPresentModeKHR presentMode)
    {
        createInfo.presentMode = presentMode;
        return *this;
    }

    Swapchain* SwapchainBuilder::Build()
    {
        return new Swapchain(createInfo);
    }
}  // namespace FooGame
