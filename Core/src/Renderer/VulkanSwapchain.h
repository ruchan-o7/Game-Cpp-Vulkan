#pragma once
#include "VulkanObject.h"

typedef struct GLFWwindow GLFWwindow;

namespace fg {

class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class Renderer;

class VulkanSwapchain {
  public:
    // clang-format off
    VulkanSwapchain(GLFWwindow* window, 
                     Renderer* renderer,
                    std::shared_ptr<VulkanInstance> instance,
                    std::shared_ptr<VulkanLogicalDevice> logicalDevice,
                    const VulkanPhysicalDevice& pDev);
    // clang-format on

    uint32_t GetFrameIndex() const {
      return m_FrameIndex;
    }
    VkExtent2D GetExtent() const {
      return m_Extent;
    }
    VkSurfaceFormatKHR Format() const {
      return m_SurfaceFormat;
    }
    VkImageView GetCurrentImageView() const {
      return m_Views[m_FrameIndex];
    }
    VkImage GetCurrentImage() const {
      return m_Images[m_FrameIndex];
    }
    VkResult AcquireNextImage();
    void Present();

  private:
    void CreateSurface();
    void CreateSwapchain();
    void RecreateSwapchain();
    void DestroySwapchainRes(bool destroySwapchain = true);
    void WaitForImageAcquiredFences();

  private:
    VkSurfaceFormatKHR m_SurfaceFormat;
    VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
    GLFWwindow* m_Window = nullptr;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    VkExtent2D m_Extent;
    std::shared_ptr<VulkanInstance> m_VkInstance;
    std::shared_ptr<VulkanLogicalDevice> m_Device;
    Renderer* m_Renderer;
    const VulkanPhysicalDevice& m_PhysicalDevice;
    std::vector<VkImageView> m_Views;
    // TODO: Remove this
    std::vector<VkImage> m_Images;
    SemaphoreWrapper m_ImageAvailable;
    SemaphoreWrapper m_RenderFinished;
    FenceWrapper m_InFlight;
    // std::vector<VkSemaphore> m_Semaphores;
    // std::vector<VkFence> m_Fences;
    uint32_t m_FrameIndex = 0;
    uint32_t m_ImageCount = 0;
};
}  // namespace fg
