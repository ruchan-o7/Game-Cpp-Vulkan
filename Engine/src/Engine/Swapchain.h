#pragma once
#include <vulkan/vulkan.h>
#include "Sync.h"
#include "Texture2D.h"
#include <vector>
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
                                      uint32_t* imageIndex);

            VkFormat GetImageFormat() const { return m_ImageFormat; };
            VkSurfaceFormatKHR GetSurfaceImageFormat() const
            {
                return m_SurfaceFormat;
            };
            uint32_t GetImageViewCount() const
            {
                return m_SwapchainImageViews.size();
            };
            inline VkImageView GetImageView(uint32_t index) const
            {
                return m_SwapchainImageViews[index];
            }
            VkPresentModeKHR GetPresentMode() const { return m_PresentMode; }
            VkExtent2D GetExtent() const { return m_Extent; }
            void Recreate(VkExtent2D extent);
            VkFramebuffer GetFrameBuffer(uint32_t imageIndex);
            void CreateFramebuffers();
            void SetRenderpass();
            void CreateDepthResources();

        private:
            VkSwapchainKHR m_Swapchain;
            VkSurfaceFormatKHR m_SurfaceFormat;
            VkPresentModeKHR m_PresentMode;
            VkExtent2D m_Extent;
            VkFormat m_ImageFormat;
            uint32_t m_ImageCount;
            std::vector<VkImage> m_SwapchainImages;
            std::vector<VkImageView> m_SwapchainImageViews;
            std::vector<VkFramebuffer> m_SwapchainFrameBuffers;
            SwapchainCreateInfo m_Info;
            Texture2D m_DepthImage;

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
