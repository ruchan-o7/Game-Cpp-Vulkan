#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../Graphics/Api.h"
#include "Device.h"
struct GLFWwindow;
namespace FooGame
{
    struct SwapchainCreateInfo
    {
            Device* device;
            VkSurfaceKHR* surface;
            VkExtent2D extent;
            VkPresentModeKHR presentMode;
            VkSurfaceFormatKHR surfaceFormat;
            VkSwapchainKHR oldSwapchain;
            GLFWwindow* window;
    };
    class Swapchain
    {
        public:
            Swapchain() = default;
            ~Swapchain();
            void Init(SwapchainCreateInfo swapchainCreateInfo);
            void Destroy();

            VkSurfaceFormatKHR GetImageFormat() const
            {
                return m_SurfaceFormat;
            };
            u32 GetImageViewCount() const
            {
                return m_SwapchainImagesViews.size();
            };
            VkImageView GetImageView(u32 index) const
            {
                return m_SwapchainImagesViews[index];
            }
            VkExtent2D GetExtent() const { return m_Extent; }

        private:
            VkSwapchainKHR m_Swapchain;
            VkSurfaceFormatKHR m_SurfaceFormat;
            VkPresentModeKHR m_PresentMode;
            VkExtent2D m_Extent;
            u32 m_ImageCount;
            List<VkImage> m_SwapchainImages;
            List<VkImageView> m_SwapchainImagesViews;
    };
    class SwapchainBuilder
    {
        public:
            SwapchainBuilder(Device* device, VkSurfaceKHR* surface);
            ~SwapchainBuilder() = default;
            SwapchainBuilder& SetOldSwapchain(VkSwapchainKHR oldSwapchain);
            Swapchain Build();

        private:
            SwapchainCreateInfo createInfo;
    };

}  // namespace FooGame
