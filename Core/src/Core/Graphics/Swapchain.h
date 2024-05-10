#pragma once
#include <vulkan/vulkan.h>
#include "../Graphics/Semaphore.h"
#include "../Graphics/Image.h"
#include "Device.h"
struct GLFWwindow;
namespace FooGame
{

    struct SwapchainCreateInfo
    {
            VkExtent2D extent;
            VkPresentModeKHR presentMode;
            VkSwapchainKHR oldSwapchain;
            VkRenderPass renderPass;
    };
    class Swapchain
    {
        public:
            Swapchain(SwapchainCreateInfo info);
            Swapchain(const Swapchain& swapchain) = delete;
            Swapchain(Swapchain&& other)          = delete;
            ~Swapchain();
            void Init();
            void Destroy();
            VkSwapchainKHR* Get() { return &m_Swapchain; }
            VkResult AcquireNextImage(VkDevice device, Semaphore& semaphore,
                                      u32* imageIndex);

            VkFormat GetImageFormat() const { return m_ImageFormat; };
            VkSurfaceFormatKHR GetSurfaceImageFormat() const
            {
                return m_SurfaceFormat;
            };
            u32 GetImageViewCount() const
            {
                return m_SwapchainImageViews.size();
            };
            inline VkImageView GetImageView(u32 index) const
            {
                return m_SwapchainImageViews[index];
            }
            VkPresentModeKHR GetPresentMode() const { return m_PresentMode; }
            VkExtent2D GetExtent() const { return m_Extent; }
            void Recreate(VkExtent2D extent);
            VkFramebuffer GetFrameBuffer(u32 imageIndex);
            void CreateFramebuffers();
            void SetRenderpass();
            void CreateDepthResources();

        private:
            VkSwapchainKHR m_Swapchain;
            VkSurfaceFormatKHR m_SurfaceFormat;
            VkPresentModeKHR m_PresentMode;
            VkExtent2D m_Extent;
            VkFormat m_ImageFormat;
            u32 m_ImageCount;
            List<VkImage> m_SwapchainImages;
            List<VkImageView> m_SwapchainImageViews;
            List<VkFramebuffer> m_SwapchainFrameBuffers;
            SwapchainCreateInfo m_Info;
            Image m_DepthImage;

        private:
    };
    class SwapchainBuilder
    {
        public:
            SwapchainBuilder();
            ~SwapchainBuilder() = default;

            SwapchainBuilder& SetExtent(VkExtent2D extent);
            SwapchainBuilder& SetOldSwapchain(
                VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
            SwapchainBuilder& SetSurfaceFormat(
                VkSurfaceFormatKHR surfaceFormat);
            SwapchainBuilder& SetPresentMode(VkPresentModeKHR presentMode);
            Swapchain* Build();

        private:
            SwapchainCreateInfo createInfo{};
    };

}  // namespace FooGame
