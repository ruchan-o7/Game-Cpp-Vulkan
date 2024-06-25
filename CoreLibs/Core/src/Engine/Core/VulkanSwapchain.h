#pragma once
#include <cstdint>
#include <memory>
#include "Utils/VulkanObjectWrapper.h"
#include "VulkanLogicalDevice.h"
#include "vulkan/vulkan_core.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace ENGINE_NAMESPACE
{
    class VulkanDeviceContext;
    class RenderDevice;
    struct ImageAcquireResult
    {
            VkResult Result;
            uint32_t ImageIndex;
    };

    struct SwapchainDescription
    {
            uint32_t Width, Height;
            VkFormat VkColorFormat                     = VK_FORMAT_R8G8B8A8_SRGB;
            VkImageUsageFlags Usage                    = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            VkSurfaceTransformFlagBitsKHR PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            uint32_t BufferCount                       = 2;
            float DefaultDepthValue                    = 1.0f;
    };

    class VulkanSwapchain
    {
            DELETE_COPY_MOVE(VulkanSwapchain);

        public:
            VulkanSwapchain(const SwapchainDescription& desc, RenderDevice* pRenderDevice,
                            VulkanDeviceContext* pDeviceContext, GLFWwindow* window);
            ~VulkanSwapchain();

        public:
            VkSurfaceKHR GetVkSurface() const { return m_VkSurface; }
            VkSwapchainKHR GetVkSwapchain() const { return m_VkSwapchain; }
            VkFormat GetImageFormat() const { return m_Desc.VkColorFormat; }
            VkImageView GetImageView(int index) const { return m_SwapchainImageViews[index]; }
            VkImageView GetDepthImageView() const { return m_DepthImageView; }
            VkExtent2D GetExtent() const { return {m_Desc.Width, m_Desc.Height}; }
            uint32_t GetBackBufferIndex() const { return m_CurrentFrame; }
            VkSemaphore GetWaitSemaphore() const;
            VkFence GetRenderFinishedFence() const;

            ImageAcquireResult QueuePresent(VkQueue presentQueue);
            VkResult QueueSubmit(VkQueue graphicsQueue, VkCommandBuffer& commandBuffer);
            ImageAcquireResult AcquireNextImage();
            void ReCreate();

        private:
            void CreateSurface();
            void CreateVkSwapchain();
            void InitBuffersAndViews();
            void RecreateVkSwapchain(VulkanDeviceContext* pDeviceContext);
            void WaitForImageAcquiredFences();
            void ReleaseSwapchainResources(VulkanDeviceContext* pDeviceContext,
                                           bool destroyVkSwapchain = false);

            bool ShouldResize(uint32_t newW, uint32_t newH);

        private:
            SwapchainDescription m_Desc;
            VkSurfaceKHR m_VkSurface;
            VkSwapchainKHR m_VkSwapchain;

            GLFWwindow* m_WindowHandle;

            uint32_t m_DesiredBufferCount = 2;

            std::vector<VkSemaphore> m_ImageAvailableSemaphores;
            std::vector<VkSemaphore> m_RenderFinishedSemaphores;
            std::vector<VkFence> m_InFlightFences;

            std::vector<VkImage> m_SwapchainImages;
            std::vector<ImageViewWrapper> m_SwapchainImageViews;

            std::vector<FramebufferWrapper> m_FrameBuffers;

            ImageWrapper m_DepthImage;
            ImageViewWrapper m_DepthImageView;
            DeviceMemoryWrapper m_DepthImageMem;

            uint32_t m_CurrentFrame = 0;
            uint32_t m_ImageIndex   = 0;

            bool m_IsMinimized  = false;
            bool m_VsyncEnabled = true;

            std::shared_ptr<RenderDevice> m_wpRenderDevice;
            VulkanDeviceContext* m_wpDeviceContext;
    };
}  // namespace ENGINE_NAMESPACE
