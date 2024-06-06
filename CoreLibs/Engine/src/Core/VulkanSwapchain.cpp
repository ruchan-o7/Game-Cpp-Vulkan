#include "Utils/VulkanObjectWrapper.h"
#include "VulkanSwapchain.h"
#include <Log.h>
#include "VulkanDeviceContext.h"
#include "RenderDevice.h"
#include <cassert>
#include <stdexcept>

namespace ENGINE_NAMESPACE
{

    VulkanSwapchain::VulkanSwapchain(const SwapchainDescription& desc,
                                     // std::shared_ptr<VulkanInstance> vkInstance,
                                     RenderDevice* pRenderDevice,
                                     VulkanDeviceContext* pDeviceContext, GLFWwindow* window)
        : m_Desc(desc),
          m_VkSurface(VK_NULL_HANDLE),
          m_VkSwapchain(VK_NULL_HANDLE),
          m_WindowHandle(window),
          m_wpRenderDevice(pRenderDevice),
          m_wpDeviceContext(pDeviceContext)
    // m_VulkanInstance(vkInstance)
    {
        CreateSurface();
        CreateVkSwapchain();
        InitBuffersAndViews();
        auto res = AcquireNextImage(pDeviceContext);
        (void)res;
    }
    void VulkanSwapchain::Present(uint32_t syncInterval)
    {
        assert((syncInterval == 0 || syncInterval == 1) &&
               "Vulkan only supports 1 and 0 for sync interval");
        auto pDeviceContext = m_wpDeviceContext;
        auto* pCtx          = pDeviceContext;
        auto pRD            = m_wpRenderDevice;
        if (!pDeviceContext)
        {
            FOO_ENGINE_ERROR("Device context has been released");
            return;
        }
        if (!m_IsMinimized)
        {
            pCtx->TransitionImageLayout();
        }
        pCtx->Flush();
        if (!m_IsMinimized)
        {
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.pNext              = nullptr;
            presentInfo.waitSemaphoreCount = 1;

            VkSemaphore waitSemaphores[] = {m_DrawCompleteSemaphores[m_SemaphoreIndex]};
            presentInfo.pWaitSemaphores  = waitSemaphores;
            presentInfo.swapchainCount   = 1;
            presentInfo.pSwapchains      = &m_VkSwapchain;
            presentInfo.pImageIndices    = &m_BackBufferIndex;

            VkResult result      = VK_SUCCESS;
            presentInfo.pResults = &result;

            result = vkQueuePresentKHR(pRD->GetLogicalDevice()->GetQueue(0, 0), &presentInfo);
            if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                RecreateVkSwapchain(pCtx);
            }
            else
            {
                FOO_ENGINE_ERROR("Present failed");
            }
            pCtx->FinishFrame();
            if (!m_IsMinimized)
            {
                ++m_SemaphoreIndex;
                if (m_SemaphoreIndex >= m_DesiredBufferCount)
                {
                    m_SemaphoreIndex = 0;
                }
                bool enableVsync = syncInterval != 0;
                auto res         = (m_VsyncEnabled == enableVsync) ? AcquireNextImage(pCtx)
                                                                   : VK_ERROR_OUT_OF_DATE_KHR;
                if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
                {
                    m_VsyncEnabled = enableVsync;
                    RecreateVkSwapchain(pCtx);
                    // TODO : Default buffer Count is 2
                    static constexpr int swapchainBufferCount = 2;

                    m_SemaphoreIndex = swapchainBufferCount - 1;

                    res = AcquireNextImage(pCtx);
                }
                assert(res == VK_SUCCESS && "Failed to acquire next swap chain");
            }
        }
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
        swapchainCreate.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreate.surface = m_VkSurface;
        swapchainCreate.minImageCount =
            surfCapabilities.minImageCount;  // TODO: this can be trouble

        swapchainCreate.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
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
        m_ImageAcquiredSemaphores.resize(imageCount);
        m_DrawCompleteSemaphores.resize(imageCount);
        m_ImageAcquiredFences.resize(imageCount);

        auto logical = pRenderDevice->GetLogicalDevice();
        for (uint32_t i = 0; i < imageCount; i++)
        {
            VkSemaphoreCreateInfo SemaphoreCI = {};

            SemaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            SemaphoreCI.pNext = nullptr;
            SemaphoreCI.flags = 0;  // reserved for future use

            auto imageAcqSem             = logical->CreateVulkanSemaphore(SemaphoreCI);
            m_ImageAcquiredSemaphores[i] = imageAcqSem;
            auto drawCompSemp            = logical->CreateVulkanSemaphore(SemaphoreCI);
            m_DrawCompleteSemaphores[i]  = drawCompSemp;

            VkFenceCreateInfo FenceCI = {};

            FenceCI.sType            = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            FenceCI.pNext            = nullptr;
            FenceCI.flags            = 0;
            m_ImageAcquiredFences[i] = logical->CreateFence(FenceCI);
        }
    }
    void VulkanSwapchain::InitBuffersAndViews()
    {
        m_SwapchainImagesInitialized.resize(2, false);
        m_ImageAcquiredFenceSubmitted.resize(2, false);
    }

    void VulkanSwapchain::WaitForImageAcquiredFences()
    {
        const auto& LogicalDevice = m_wpRenderDevice->GetLogicalDevice();
        for (size_t i = 0; i < m_ImageAcquiredFences.size(); ++i)
        {
            if (m_ImageAcquiredFenceSubmitted[i])
            {
                VkFence vkFence = m_ImageAcquiredFences[i];
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
        auto pRenderDevice   = m_wpRenderDevice;
        const auto pDeviceVk = pRenderDevice->GetLogicalDevice();
        auto oldestSubmittedImageFenceIndex =
            (m_SemaphoreIndex + 1u) % static_cast<uint32_t>(m_ImageAcquiredFenceSubmitted.size());
        if (m_ImageAcquiredFenceSubmitted[oldestSubmittedImageFenceIndex])
        {
            VkFence oldestSubmittedFence = m_ImageAcquiredFences[oldestSubmittedImageFenceIndex];
            if (pDeviceVk->GetFenceStatus(oldestSubmittedFence) == VK_NOT_READY)
            {
                auto res = pDeviceVk->WaitForFences(1, &oldestSubmittedFence, VK_TRUE, UINT64_MAX);
                assert(res == VK_SUCCESS);
                (void)res;
            }
            pDeviceVk->ResetFence(oldestSubmittedFence);
            m_ImageAcquiredFenceSubmitted[oldestSubmittedImageFenceIndex] = false;
        }

        VkFence imageAcquiredFence         = m_ImageAcquiredFences[m_SemaphoreIndex];
        VkSemaphore imageAcquiredSemaphore = m_ImageAcquiredSemaphores[m_SemaphoreIndex];

        auto res =
            vkAcquireNextImageKHR(pDeviceVk->GetVkDevice(), m_VkSwapchain, UINT64_MAX,
                                  imageAcquiredSemaphore, imageAcquiredFence, &m_BackBufferIndex);
        m_ImageAcquiredFenceSubmitted[m_SemaphoreIndex] = (res == VK_SUCCESS);

        if (res == VK_SUCCESS)
        {
            pDeviceContext->AddWaitSemaphore(
                &m_ImageAcquiredSemaphores[m_SemaphoreIndex],
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT);
            if (!m_SwapchainImagesInitialized[m_BackBufferIndex])
            {
                // TODO
            }
        }
        return res;
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
            pDeviceContext->Flush();
            bool renderTargetsReset = false;
        }
        pDeviceContext->IdleGPU();
        WaitForImageAcquiredFences();

        m_SwapchainImagesInitialized.clear();
        m_ImageAcquiredFenceSubmitted.clear();

        m_ImageAcquiredSemaphores.clear();
        m_DrawCompleteSemaphores.clear();
        m_ImageAcquiredFences.clear();
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
