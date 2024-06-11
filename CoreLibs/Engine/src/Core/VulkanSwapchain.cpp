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
        auto res = AcquireNextImage(pDeviceContext);
        (void)res;
    }
    void VulkanSwapchain::Present(uint32_t syncInterval, VkCommandBuffer& cmd)
    {
        VkSemaphore waitSemaphores[]      = {m_ImageAvailableSemaphores[syncInterval]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores    = waitSemaphores;
        submitInfo.pWaitDstStageMask  = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &cmd;

        VkSemaphore signalSemaphores[]  = {m_RenderFinishedSemaphores[syncInterval]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        auto err = vkQueueSubmit(m_wpRenderDevice->GetGraphicsQueue(), 1, &submitInfo,
                                 m_InFlightFences[syncInterval]);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;

        VkSwapchainKHR swapChains[] = {m_VkSwapchain};
        presentInfo.swapchainCount  = 1;
        presentInfo.pSwapchains     = swapChains;

        presentInfo.pImageIndices = &(m_BackBufferIndex);

        err = vkQueuePresentKHR(m_wpRenderDevice->GetGraphicsQueue(), &presentInfo);

        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            RecreateVkSwapchain(m_wpDeviceContext);
        }
        else if (err != VK_SUCCESS)
        {
            FOO_ENGINE_CRITICAL("Failed to presenet swap chain images");
        }
        AcquireNextImage(m_wpDeviceContext);
    }
    void VulkanSwapchain::ResetFences(uint32_t index)
    {
        vkResetFences(m_wpRenderDevice->GetVkDevice(), 1, &m_InFlightFences[index]);
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
    VkResult VulkanSwapchain::AcquireNextImage(uint32_t* imageIndex, uint32_t currentFrame)
    {
        auto device = m_wpRenderDevice->GetVkDevice();
        vkWaitForFences(device, 1, &m_InFlightFences[currentFrame], VK_TRUE, UINT8_MAX);
        VkResult result   = vkAcquireNextImageKHR(device, m_VkSwapchain, UINT8_MAX,
                                                  m_ImageAvailableSemaphores[currentFrame],
                                                  VK_NULL_HANDLE, imageIndex);
        m_BackBufferIndex = *imageIndex;
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            ReCreate();
            return VK_ERROR_OUT_OF_DATE_KHR;
        }
        vkResetFences(device, 1, &m_InFlightFences[currentFrame]);
        return result;
    }
    VkResult VulkanSwapchain::QueueSubmit(VkQueue graphicsQueue, uint32_t currentFrame,
                                          VkCommandBuffer& commandBuffer)
    {
        VkSemaphore waitSemaphores[]      = {m_ImageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[]    = {m_RenderFinishedSemaphores[currentFrame]};
        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores    = waitSemaphores;
        submitInfo.pWaitDstStageMask  = waitStages;

        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;
        return vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_InFlightFences[currentFrame]);
    }
    VkResult VulkanSwapchain::QueuePresent(VkQueue presentQueue, uint32_t currentFrame)
    {
        VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[currentFrame]};
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;

        VkSwapchainKHR swapchain[] = {m_VkSwapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains    = swapchain;

        presentInfo.pImageIndices = &m_BackBufferIndex;
        return vkQueuePresentKHR(presentQueue, &presentInfo);
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
        auto oldSwapchain    = m_VkSwapchain;
        m_Desc.VkColorFormat = VK_FORMAT_R8G8B8A8_UNORM;

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
            FenceCI.flags = 0;
            vkCreateFence(logical->GetVkDevice(), &FenceCI, nullptr, &m_InFlightFences[i]);
        }
    }
    void VulkanSwapchain::InitBuffersAndViews()
    {
        auto logicalVkDev = m_wpRenderDevice->GetVkDevice();
        auto& lDevice     = m_wpRenderDevice->GetLogicalDevice();
        m_SwapchainImagesInitialized.resize(2, false);
        m_ImageAcquiredFenceSubmitted.resize(2, false);

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

        assert(err == VK_SUCCESS);
    }

    void VulkanSwapchain::WaitForImageAcquiredFences()
    {
        const auto& LogicalDevice = m_wpRenderDevice->GetLogicalDevice();
        for (size_t i = 0; i < m_InFlightFences.size(); ++i)
        {
            if (m_ImageAcquiredFenceSubmitted[i])
            {
                VkFence vkFence = m_InFlightFences[i];
                if (LogicalDevice->GetFenceStatus(vkFence) == VK_NOT_READY)
                {
                    LogicalDevice->WaitForFences(1, &vkFence, VK_TRUE, UINT64_MAX);
                }
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
    void VulkanSwapchain::Resize(uint32_t newWidth, uint32_t newHeight)
    {
        bool recreateSwapchain = false;
        if (ShouldResize(newWidth, newHeight))
        {
            recreateSwapchain = true;
        }
        if (recreateSwapchain)
        {
            auto pDevCtx = m_wpDeviceContext;
            assert(pDevCtx && "Context has been released ");
            if (pDevCtx)
            {
                try
                {
                    auto* pRawDevCtx = pDevCtx;
                    RecreateVkSwapchain(pRawDevCtx);
                    auto res = AcquireNextImage(pRawDevCtx);
                    (void)res;
                }
                catch (const std::runtime_error&)
                {
                    FOO_ENGINE_ERROR("Failed to resize swap chain");
                }
            }
        }
        m_IsMinimized = (newWidth == 0 && newHeight == 0);
    }
    VkResult VulkanSwapchain::AcquireNextImage(class VulkanDeviceContext* pDeviceContext)
    {
        auto err = vkAcquireNextImageKHR(
            m_wpDeviceContext->GetVkDevice()->GetVkDevice(), m_VkSwapchain, UINT8_MAX,
            m_ImageAvailableSemaphores[m_SyncInterval], 0, &m_BackBufferIndex);
        return err;

        // auto pRenderDevice   = m_wpRenderDevice;
        // const auto pDeviceVk = pRenderDevice->GetLogicalDevice();
        // vkWaitForFences(pDeviceVk->GetVkDevice(), 1, &m_InFlightFences[m_BackBufferIndex],
        // VK_TRUE, UINT8_MAX);
        // // auto oldestSubmittedImageFenceIndex =
        // //     (m_SemaphoreIndex + 1u) %
        // static_cast<uint32_t>(m_ImageAcquiredFenceSubmitted.size());
        // // if (m_ImageAcquiredFenceSubmitted[oldestSubmittedImageFenceIndex])
        // // {
        // //     VkFence oldestSubmittedFence = m_InFlightFences[oldestSubmittedImageFenceIndex];
        // //     if (pDeviceVk->GetFenceStatus(oldestSubmittedFence) == VK_NOT_READY)
        // //     {
        // //         auto res = pDeviceVk->WaitForFences(1, &oldestSubmittedFence, VK_TRUE,
        // UINT64_MAX);
        // //         assert(res == VK_SUCCESS);
        // //         (void)res;
        // //     }
        // //     pDeviceVk->ResetFence(oldestSubmittedFence);
        // //     m_ImageAcquiredFenceSubmitted[oldestSubmittedImageFenceIndex] = false;
        // // }

        // auto res =
        //     vkAcquireNextImageKHR(pDeviceVk->GetVkDevice(), m_VkSwapchain, UINT64_MAX,
        //                           m_ImageAvailableSemaphores[m_BackBufferIndex], VK_NULL_HANDLE,
        //                           &m_BackBufferIndex);
        // m_ImageAcquiredFenceSubmitted[m_SemaphoreIndex] = (res == VK_SUCCESS);

        // if (res == VK_SUCCESS)
        // {
        //     pDeviceContext->AddWaitSemaphore(
        //         &m_ImageAvailableSemaphores[m_SemaphoreIndex],
        //         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT);
        //     if (!m_SwapchainImagesInitialized[m_BackBufferIndex])
        //     {
        //         m_SwapchainImagesInitialized[m_BackBufferIndex] = true;
        //         // TODO
        //     }
        // }
        // vkResetFences(pDeviceVk->GetVkDevice(), 1, &m_InFlightFences[m_BackBufferIndex]);
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
        pDeviceContext->IdleGPU();
        WaitForImageAcquiredFences();

        m_SwapchainImagesInitialized.clear();
        m_ImageAcquiredFenceSubmitted.clear();

        m_ImageAvailableSemaphores.clear();
        m_RenderFinishedSemaphores.clear();
        m_InFlightFences.clear();
        m_SemaphoreIndex = 0;

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
