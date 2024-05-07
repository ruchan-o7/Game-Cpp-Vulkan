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
            Shared<Device> device;
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
            Swapchain(const Swapchain& swapchain) = delete;
            Swapchain(Swapchain&& other)          = delete;
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
                return m_SwapchainImageViews.size();
            };
            inline VkImageView GetImageView(u32 index) const
            {
                return m_SwapchainImageViews[index];
            }
            VkExtent2D GetExtent() const { return m_Extent; }
            void Recreate(VkExtent2D extent);
            VkFramebuffer GetFrameBuffer(u32 imageIndex);
            void CreateFramebuffers();
            void SetRenderpass(VkRenderPass* renderPass);
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
            SwapchainBuilder(Shared<Device> device, VkSurfaceKHR& surface);
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
