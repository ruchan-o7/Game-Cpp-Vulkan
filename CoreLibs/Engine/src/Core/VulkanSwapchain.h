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
            void Present(uint32_t syncInterval, VkCommandBuffer& cmd);
            void Resize(uint32_t newWidth, uint32_t newHeight);
            VkSurfaceKHR GetVkSurface() const { return m_VkSurface; }
            VkSwapchainKHR GetVkSwapchain() const { return m_VkSwapchain; }
            VkFormat GetImageFormat() const { return m_Desc.VkColorFormat; }
            VkImageView GetImageView(int index) const { return m_SwapchainImageViews[index]; }
            VkImageView GetDepthImageView() const { return m_DepthImageView; }
            VkExtent2D GetExtent() const { return {m_Desc.Width, m_Desc.Height}; }
            uint32_t GetBackBufferIndex() const { return m_CurrentFrame; }

            ImageAcquireResult QueuePresent(VkQueue presentQueue);
            VkResult QueueSubmit(VkQueue graphicsQueue, uint32_t currentFrame,
                                 VkCommandBuffer& commandBuffer);
            ImageAcquireResult AcquireNextImage();
            void ReCreate();

        private:
            void CreateSurface();
            void CreateVkSwapchain();
            void InitBuffersAndViews();
            // VkResult AcquireNextImage(VulkanDeviceContext* pDeviceContext);
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

            std::vector<bool> m_SwapchainImagesInitialized;
            std::vector<bool> m_ImageAcquiredFenceSubmitted;

            std::vector<VkImage> m_SwapchainImages;
            std::vector<ImageViewWrapper> m_SwapchainImageViews;

            ImageWrapper m_DepthImage;
            ImageViewWrapper m_DepthImageView;
            DeviceMemoryWrapper m_DepthImageMem;

            // std::vector<FramebufferWrapper> m_FrameBuffers;

            uint32_t m_SemaphoreIndex = 0;
            uint32_t m_CurrentFrame   = 0;
            uint32_t m_ImageIndex     = 0;
            uint32_t m_SyncInterval   = 0;

            bool m_IsMinimized  = false;
            bool m_VsyncEnabled = true;

            std::shared_ptr<RenderDevice> m_wpRenderDevice;
            VulkanDeviceContext* m_wpDeviceContext;
    };
}  // namespace ENGINE_NAMESPACE
