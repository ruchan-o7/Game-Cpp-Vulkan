#pragma once
#include <vulkan/vulkan.h>
#include "../Graphics/Semaphore.h"
#include "Device.h"
struct GLFWwindow;
namespace FooGame
{
    struct SwapchainCreateInfo
    {
            Device* device;
            VkSurfaceKHR surface;
            VkExtent2D extent;
            VkPresentModeKHR presentMode;
            VkSwapchainKHR oldSwapchain;
            VkRenderPass* renderPass;
    };
    class Swapchain
    {
        public:
            Swapchain(SwapchainCreateInfo info);
            ~Swapchain();
            void Init();
            void Destroy();
            VkSwapchainKHR* Get() { return &m_Swapchain; }
            VkResult AcquireNextImage(VkDevice device, Semaphore& semaphore,
                                      u32* imageIndex);

            VkSurfaceFormatKHR GetSurfaceImageFormat() const
            {
                return m_SurfaceFormat;
            };
            VkFormat GetImageFormat() const { return m_ImageFormat; };
            u32 GetImageViewCount() const
            {
                return m_SwapchainImagesViews.size();
            };
            VkImageView GetImageView(u32 index) const
            {
                return m_SwapchainImagesViews[index];
            }
            VkExtent2D GetExtent() const { return m_Extent; }
            void Recreate(VkExtent2D extent);
            VkFramebuffer GetFrameBuffer(u32 imageIndex);
            void CreateFramebuffers();
            void SetRenderpass(VkRenderPass* renderPass);

        private:
            VkSwapchainKHR m_Swapchain;
            VkSurfaceFormatKHR m_SurfaceFormat;
            VkPresentModeKHR m_PresentMode;
            VkExtent2D m_Extent;
            VkFormat m_ImageFormat;
            u32 m_ImageCount;
            List<VkImage> m_SwapchainImages;
            List<VkImageView> m_SwapchainImagesViews;
            List<VkFramebuffer> m_SwapchainFrameBuffers;
            SwapchainCreateInfo m_SwapchainCreateInfo;
    };
    class SwapchainBuilder
    {
        public:
            SwapchainBuilder(Device& device, VkSurfaceKHR& surface);
            ~SwapchainBuilder() = default;

            SwapchainBuilder& SetExtent(VkExtent2D extent);
            SwapchainBuilder& SetOldSwapchain(
                VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
            SwapchainBuilder& SetSurface(VkSurfaceKHR surface);
            SwapchainBuilder& SetSurfaceFormat(
                VkSurfaceFormatKHR surfaceFormat);
            SwapchainBuilder& SetPresentMode(VkPresentModeKHR presentMode);
            Shared<Swapchain> Build();

        private:
            SwapchainCreateInfo createInfo{};
    };

}  // namespace FooGame
