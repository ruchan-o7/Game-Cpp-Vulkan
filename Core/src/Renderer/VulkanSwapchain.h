#pragma once
#include "vulkan/vulkan.h"

typedef struct GLFWwindow GLFWwindow;

namespace fg {

class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanPhysicalDevice;
class VulkanLogicalDevice;

class VulkanSwapchain {
  public:
    // clang-format off
    VulkanSwapchain(GLFWwindow* window, 
                    std::shared_ptr<VulkanInstance> instance,
                    std::shared_ptr<VulkanLogicalDevice> logicalDevice,
                    const VulkanPhysicalDevice& pDev);
    // clang-format on

    uint32_t GetFrameIndex() const {
      return m_FrameIndex;
    }

  private:
    void CreateSurface();
    void CreateSwapchain();

  private:
    VkSurfaceFormatKHR m_SurfaceFormat;
    VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
    GLFWwindow* m_Window = nullptr;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    std::shared_ptr<VulkanInstance> m_VkInstance;
    std::shared_ptr<VulkanLogicalDevice> m_Device;
    const VulkanPhysicalDevice& m_PhysicalDevice;
    std::vector<VkImageView> m_Views;
    std::vector<VkSemaphore> m_Semaphores;
    std::vector<VkFence> m_Fences;
    uint32_t m_FrameIndex = 0;
    uint32_t m_ImageCount = 0;
};
}  // namespace fg
