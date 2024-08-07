#include "VulkanSwapchain.h"
#include <Log.h>
#include "VulkanDeviceContext.h"
#include "RenderDevice.h"
#include "VulkanLogicalDevice.h"
#include "../Engine/Backend.h"
#include "src/Log.h"
#include "vulkan/vulkan_core.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace ENGINE_NAMESPACE
{

    VulkanSwapchain::VulkanSwapchain(const SwapchainDescription& desc, RenderDevice* pRenderDevice,
                                     VulkanDeviceContext* pDeviceContext, GLFWwindow* window)
        : m_Desc(desc),
          m_VkSurface(VK_NULL_HANDLE),
          m_VkSwapchain(VK_NULL_HANDLE),
          m_WindowHandle(window),
          m_wpRenderDevice(pRenderDevice),
          m_wpDeviceContext(pDeviceContext)
    {
        CreateSurface();
        CreateVkSwapchain();
        InitBuffersAndViews();
    }

    void VulkanSwapchain::ReCreate()
    {
        ReleaseSwapchainResources(m_wpDeviceContext, false);
        {
            auto pRenderDevice        = m_wpRenderDevice;
            const auto vkDeviceHandle = pRenderDevice->GetPhysicalDevice()->GetVkDeviceHandle();

            VkSurfaceCapabilitiesKHR surfaceCaps{};
            auto err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkDeviceHandle, m_VkSurface,
                                                                 &surfaceCaps);
            if (err == VK_ERROR_SURFACE_LOST_KHR)
            {
                if (m_VkSwapchain != VK_NULL_HANDLE)
                {
                    vkDestroySwapchainKHR(pRenderDevice->GetVkDevice(), m_VkSwapchain, nullptr);
                    m_VkSwapchain = VK_NULL_HANDLE;
                }
                CreateSurface();
            }
        }
        CreateVkSwapchain();
        InitBuffersAndViews();
    }
    ImageAcquireResult VulkanSwapchain::AcquireNextImage()
    {
        auto device = m_wpRenderDevice->GetVkDevice();
        VkResult result =
            vkWaitForFences(device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT32_MAX);

        result = vkAcquireNextImageKHR(device, m_VkSwapchain, UINT32_MAX,
                                       m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE,
                                       &m_ImageIndex);
        if (result != VK_SUCCESS)
        {
            FOO_ENGINE_ERROR("Result is not succes");
        }
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            ReCreate();
            return {VK_ERROR_OUT_OF_DATE_KHR, 0};
        }
        vkResetFences(device, 1, &m_InFlightFences[m_CurrentFrame]);
        return {result, m_ImageIndex};
    }
    VkSemaphore VulkanSwapchain::GetWaitSemaphore() const
    {
        return m_ImageAvailableSemaphores[m_CurrentFrame];
    }
    VkFence VulkanSwapchain::GetRenderFinishedFence() const
    {
        return m_InFlightFences[m_ImageIndex];
    }

    VkResult VulkanSwapchain::QueueSubmit(VkQueue graphicsQueue, VkCommandBuffer& commandBuffer)
    {
        VkSemaphore waitSemaphores[]      = {m_ImageAvailableSemaphores[m_CurrentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[]    = {m_RenderFinishedSemaphores[m_CurrentFrame]};

        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores    = waitSemaphores;
        submitInfo.pWaitDstStageMask  = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        return vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
    }
    ImageAcquireResult VulkanSwapchain::QueuePresent(VkQueue presentQueue)
    {
        VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;

        VkSwapchainKHR swapchain[] = {m_VkSwapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains    = swapchain;

        presentInfo.pImageIndices = &m_ImageIndex;

        auto err = vkQueuePresentKHR(presentQueue, &presentInfo);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            ReCreate();
        }
        else if (err != VK_SUCCESS)
        {
            FOO_ENGINE_ERROR("Failed to present swap chain");
        }
        m_CurrentFrame = (m_CurrentFrame + 1) % m_Desc.BufferCount;
        return {err, m_CurrentFrame};
    }

    void VulkanSwapchain::RecreateVkSwapchain(VulkanDeviceContext* pDeviceContext)
    {
        ReleaseSwapchainResources(pDeviceContext, false);
        {
            auto pRenderDevice        = m_wpRenderDevice;
            const auto vkDeviceHandle = pRenderDevice->GetPhysicalDevice()->GetVkDeviceHandle();

            VkSurfaceCapabilitiesKHR surfaceCaps{};
            auto err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkDeviceHandle, m_VkSurface,
                                                                 &surfaceCaps);
            if (err == VK_ERROR_SURFACE_LOST_KHR)
            {
                if (m_VkSwapchain != VK_NULL_HANDLE)
                {
                    vkDestroySwapchainKHR(pRenderDevice->GetVkDevice(), m_VkSwapchain, nullptr);
                    m_VkSwapchain = VK_NULL_HANDLE;
                }
                CreateSurface();
            }
        }
        CreateVkSwapchain();
        InitBuffersAndViews();
    }
    void VulkanSwapchain::CreateVkSwapchain()
    {
        auto pRenderDevice    = m_wpRenderDevice;
        const auto& physicalD = pRenderDevice->GetPhysicalDevice();
        auto pDevHandle       = physicalD->GetVkDeviceHandle();

        uint32_t formatCount = 0;

        auto err =
            vkGetPhysicalDeviceSurfaceFormatsKHR(pDevHandle, m_VkSurface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> supportedFormats(formatCount);
        err = vkGetPhysicalDeviceSurfaceFormatsKHR(pDevHandle, m_VkSurface, &formatCount,
                                                   supportedFormats.data());
        VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        VkSurfaceCapabilitiesKHR surfCapabilities = {};
        err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevHandle, m_VkSurface, &surfCapabilities);
        uint32_t presentModeCount = 0;
        err = vkGetPhysicalDeviceSurfacePresentModesKHR(pDevHandle, m_VkSurface, &presentModeCount,
                                                        NULL);
        assert(presentModeCount > 0);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        err = vkGetPhysicalDeviceSurfacePresentModesKHR(pDevHandle, m_VkSurface, &presentModeCount,
                                                        presentModes.data());
        assert(presentModeCount == presentModes.size());

        VkExtent2D extent{};
        if (surfCapabilities.currentExtent.width == 0xFFFFFFFF && m_Desc.Width != 0 &&
            m_Desc.Height != 0)
        {
            extent.width = std::min(std::max(m_Desc.Width, surfCapabilities.minImageExtent.width),
                                    surfCapabilities.maxImageExtent.width);
            extent.height =
                std::min(std::max(m_Desc.Height, surfCapabilities.minImageExtent.height),
                         surfCapabilities.maxImageExtent.height);
        }
        else
        {
            extent = surfCapabilities.currentExtent;
        }

        extent.width  = std::max(extent.width, 1u);
        extent.height = std::max(extent.height, 1u);
        m_Desc.Width  = extent.width;
        m_Desc.Height = extent.height;

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        {
            std::vector<VkPresentModeKHR> preferredPresentModes;
            if (m_VsyncEnabled)
            {
                preferredPresentModes.push_back(VK_PRESENT_MODE_FIFO_RELAXED_KHR);
                preferredPresentModes.push_back(VK_PRESENT_MODE_FIFO_KHR);
            }
            else
            {
                preferredPresentModes.push_back(VK_PRESENT_MODE_MAILBOX_KHR);
                preferredPresentModes.push_back(VK_PRESENT_MODE_IMMEDIATE_KHR);
                preferredPresentModes.push_back(VK_PRESENT_MODE_FIFO_KHR);
            }
            for (auto preferred : preferredPresentModes)
            {
                if (std::find(presentModes.begin(), presentModes.end(), preferred) !=
                    presentModes.end())
                {
                    presentMode = preferred;
                    break;
                }
            }
        }
        auto oldSwapchain = m_VkSwapchain;

        VkSwapchainCreateInfoKHR swapchainCreate{};
        swapchainCreate.sType         = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreate.surface       = m_VkSurface;
        swapchainCreate.minImageCount = surfCapabilities.minImageCount;
        swapchainCreate.imageFormat   = m_Desc.VkColorFormat;

        if (surfCapabilities.maxImageCount > 0 &&
            swapchainCreate.minImageCount > surfCapabilities.maxImageCount)
        {
            swapchainCreate.minImageCount = surfCapabilities.maxImageCount;
        }
        swapchainCreate.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainCreate.imageExtent      = extent;
        swapchainCreate.imageArrayLayers = 1;
        swapchainCreate.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        swapchainCreate.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreate.preTransform     = surfCapabilities.currentTransform;
        swapchainCreate.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreate.presentMode      = presentMode;
        swapchainCreate.clipped          = VK_TRUE;
        swapchainCreate.oldSwapchain     = oldSwapchain;

        err = vkCreateSwapchainKHR(pRenderDevice->GetVkDevice(), &swapchainCreate, nullptr,
                                   &m_VkSwapchain);
        if (oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(pRenderDevice->GetVkDevice(), oldSwapchain, nullptr);
            oldSwapchain = nullptr;
        }
        uint32_t imageCount = 0;
        err = vkGetSwapchainImagesKHR(pRenderDevice->GetVkDevice(), m_VkSwapchain, &imageCount,
                                      nullptr);
        assert(imageCount > 0);

        if (imageCount != m_Desc.BufferCount)
        {
            m_Desc.BufferCount = imageCount;
        }
        m_ImageAvailableSemaphores.resize(imageCount);
        m_RenderFinishedSemaphores.resize(imageCount);
        m_InFlightFences.resize(imageCount);

        auto logical = pRenderDevice->GetLogicalDevice();
        for (uint32_t i = 0; i < imageCount; i++)
        {
            VkSemaphoreCreateInfo SemaphoreCI = {};

            SemaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            SemaphoreCI.pNext = nullptr;
            SemaphoreCI.flags = 0;  // reserved for future use

            vkCreateSemaphore(logical->GetVkDevice(), &SemaphoreCI, nullptr,
                              &m_ImageAvailableSemaphores[i]);
            vkCreateSemaphore(logical->GetVkDevice(), &SemaphoreCI, nullptr,
                              &m_RenderFinishedSemaphores[i]);

            VkFenceCreateInfo FenceCI = {};

            FenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            FenceCI.pNext = nullptr;
            FenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkCreateFence(logical->GetVkDevice(), &FenceCI, nullptr, &m_InFlightFences[i]);
        }
    }
    void VulkanSwapchain::InitBuffersAndViews()
    {
        auto logicalVkDev = m_wpRenderDevice->GetVkDevice();
        auto& lDevice     = m_wpRenderDevice->GetLogicalDevice();

        uint32_t imageCount = m_Desc.BufferCount;

        m_SwapchainImageViews.resize(imageCount);
        m_SwapchainImages.resize(imageCount);
        auto err = vkGetSwapchainImagesKHR(logicalVkDev, m_VkSwapchain, &imageCount,
                                           m_SwapchainImages.data());

        assert(err == VK_SUCCESS);
        assert(imageCount == m_SwapchainImages.size());

        for (int i = 0; i < imageCount; i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image                           = m_SwapchainImages[i];
            viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format                          = m_Desc.VkColorFormat;
            viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel   = 0;
            viewInfo.subresourceRange.levelCount     = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount     = 1;

            m_SwapchainImageViews[i] = lDevice->CreateImageView(viewInfo);
        }
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        VkImageCreateInfo imageInfo{};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = m_Desc.Width;
        imageInfo.extent.height = m_Desc.Height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = depthFormat;
        imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

        m_DepthImage = lDevice->CreateImage(imageInfo);

        VkMemoryRequirements memRequirements =
            m_wpRenderDevice->GetLogicalDevice()->GetImageMemoryRequirements(m_DepthImage);
        VkPhysicalDeviceMemoryProperties props =
            m_wpRenderDevice->GetPhysicalDevice()->GetMemoryProperties();
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        uint32_t filter = memRequirements.memoryTypeBits;
        for (uint32_t i = 0; i < props.memoryTypeCount; i++)
        {
            if ((filter & (1 << i)) &&
                (props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                filter = i;
                break;
            }
        }

        allocInfo.memoryTypeIndex = m_wpRenderDevice->GetPhysicalDevice()->GetMemoryTypeIndex(
            memRequirements.memoryTypeBits, filter);

        m_DepthImageMem = lDevice->AllocateDeviceMemory(allocInfo);
        assert(err == VK_SUCCESS);

        lDevice->BindImageMemory(m_DepthImage, m_DepthImageMem, 0);
        assert(err == VK_SUCCESS);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = m_DepthImage;
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                          = depthFormat;
        viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        m_DepthImageView = lDevice->CreateImageView(viewInfo);
        m_FrameBuffers.resize(m_Desc.BufferCount);

        assert(err == VK_SUCCESS);
    }

    void VulkanSwapchain::WaitForImageAcquiredFences()
    {
        const auto& LogicalDevice = m_wpRenderDevice->GetLogicalDevice();
        for (size_t i = 0; i < m_InFlightFences.size(); ++i)
        {
            VkFence vkFence = m_InFlightFences[i];
            if (LogicalDevice->GetFenceStatus(vkFence) == VK_NOT_READY)
            {
                LogicalDevice->WaitForFences(1, &vkFence, VK_TRUE, UINT64_MAX);
            }
        }
    }

    bool VulkanSwapchain::ShouldResize(uint32_t newW, uint32_t newH)
    {
        bool shouldResize = false;
        if (newW != 0 && newH != 0 &&
            (m_Desc.Width != newW || m_Desc.Height != newH))  // check for equal with description
        {
            m_Desc.Width  = newW;
            m_Desc.Height = newH;
            shouldResize  = true;
        }
        return shouldResize;
    }

    void VulkanSwapchain::CreateSurface()
    {
        auto pRenderDevice = m_wpRenderDevice;
        if (m_VkSurface != nullptr)
        {
            vkDestroySurfaceKHR(pRenderDevice->GetVkInstance(), m_VkSurface, NULL);
            m_VkSurface = VK_NULL_HANDLE;
        }

        VkResult err = VK_ERROR_INITIALIZATION_FAILED;

        err = glfwCreateWindowSurface(pRenderDevice->GetVkInstance(), m_WindowHandle, nullptr,
                                      &m_VkSurface);
    }

    void VulkanSwapchain::ReleaseSwapchainResources(VulkanDeviceContext* pDeviceContext,
                                                    bool destroyVkSwapchain)
    {
        if (m_VkSwapchain == nullptr)
        {
            return;
        }
        if (pDeviceContext != nullptr)
        {
            // pDeviceContext->Flush();
            // bool renderTargetsReset = false;
        }
        m_wpRenderDevice->IdleGPU();
        WaitForImageAcquiredFences();

        m_ImageAvailableSemaphores.clear();
        m_RenderFinishedSemaphores.clear();
        m_InFlightFences.clear();

        if (destroyVkSwapchain)
        {
            vkDestroySwapchainKHR(pDeviceContext->GetVkDevice()->GetVkDevice(), m_VkSwapchain,
                                  nullptr);
        }
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        auto pDevCtx = m_wpDeviceContext;
        if (m_VkSwapchain != VK_NULL_HANDLE)
        {
            auto* pCtx = pDevCtx;
            ReleaseSwapchainResources(pCtx, true);
            assert(m_VkSwapchain == VK_NULL_HANDLE);
        }
        if (m_VkSurface != VK_NULL_HANDLE)
        {
            auto rd = m_wpRenderDevice;
            vkDestroySurfaceKHR(rd->GetVkInstance(), m_VkSurface, nullptr);
        }
    }
}  // namespace ENGINE_NAMESPACE
