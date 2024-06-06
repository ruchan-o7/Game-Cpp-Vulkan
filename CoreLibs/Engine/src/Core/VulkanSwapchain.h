#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanInstance.h"

namespace ENGINE_NAMESPACE
{
    class VulkanDeviceContext;
    class RenderDevice;
    struct SwapchainDescription
    {
            uint32_t Width, Height;
            VkFormat VkColorFormat                     = VK_FORMAT_UNDEFINED;
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
            void Present(uint32_t syncInterval);
            void Resize(uint32_t newWidth, uint32_t newHeight);
            VkSurfaceKHR GetVkSurface() const { return m_VkSurface; }
            VkSwapchainKHR GetVkSwapchain() const { return m_VkSwapchain; }

        private:
            void CreateSurface();
            void CreateVkSwapchain();
            void InitBuffersAndViews();
            VkResult AcquireNextImage(class VulkanDeviceContext* pDeviceContext);
            void RecreateVkSwapchain(class VulkanDeviceContext* pDeviceContext);
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

            std::vector<VkSemaphore> m_ImageAcquiredSemaphores;
            std::vector<VkSemaphore> m_DrawCompleteSemaphores;
            std::vector<VkFence> m_ImageAcquiredFences;

            std::vector<bool> m_SwapchainImagesInitialized;
            std::vector<bool> m_ImageAcquiredFenceSubmitted;

            uint32_t m_SemaphoreIndex  = 0;
            uint32_t m_BackBufferIndex = 0;

            bool m_IsMinimized  = false;
            bool m_VsyncEnabled = true;

            RenderDevice* m_wpRenderDevice;
            VulkanDeviceContext* m_wpDeviceContext;
    };
}  // namespace ENGINE_NAMESPACE
