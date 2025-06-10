#pragma once
#include "VulkanObject.h"
#include "../Core/Ref.h"

typedef struct GLFWwindow GLFWwindow;

namespace fg {

class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class Renderer;
class VulkanImageView;
class RenderContext;

class VulkanSwapchain : public RefBase {
  public:
    // clang-format off
    VulkanSwapchain(GLFWwindow* window, 
                    std::weak_ptr<Renderer> renderer,
                    std::shared_ptr<VulkanInstance> instance,
                    std::shared_ptr<VulkanLogicalDevice> logicalDevice,
                    const VulkanPhysicalDevice& pDev);
    VulkanSwapchain(GLFWwindow* window, 
                    Ref<Renderer> renderer,
                    WeakRef<RenderContext> context);
    // clang-format on
    ~VulkanSwapchain();

    uint32_t GetFrameIndex() const {
      return m_FrameIndex;
    }
    VkExtent2D GetExtent() const {
      return m_Extent;
    }
    VkSurfaceFormatKHR Format() const {
      return m_SurfaceFormat;
    }
    Ref<VulkanImageView> GetCurrentImageView() const {
      return m_BackbufferRtvs[m_FrameIndex];
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
    Ref<Renderer> m_Renderer;
    WeakRef<RenderContext> m_Ctx;

    VkSurfaceFormatKHR m_SurfaceFormat;
    VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
    GLFWwindow* m_Window = nullptr;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    VkExtent2D m_Extent;
    std::vector<Ref<VulkanImageView>> m_BackbufferRtvs;

    SemaphoreWrapper m_ImageAvailable;
    SemaphoreWrapper m_RenderFinished;
    FenceWrapper m_InFlight;

    uint32_t m_FrameIndex = 0;
    uint32_t m_ImageCount = 0;
};
}  // namespace fg
