#pragma once
#include "VulkanHeader.h"
#include "../Core/Ref.h"

typedef struct GLFWwindow GLFWwindow;

namespace fg {

class VulkanSwapchain;
class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class VulkanQueue;

class Renderer;
class RenderContext;

struct EngineInfo {
    GLFWwindow* WindowHandle = nullptr;
    VkAllocationCallbacks* Allocator = nullptr;
};

class RendererFactory {
  public:
    static RendererFactory& Get();

    // clang-format off
    void CreateDeviceAndContexts(
        const EngineInfo& info, 
        Ref<Renderer>* renderer,
        Ref<RenderContext>* context);
    // clang-format on

    Ref<VulkanSwapchain> CreateSwapchain(Ref<Renderer> renderer, Ref<RenderContext> ctx,
                                         GLFWwindow* window);

  private:
    // clang-format off
    void AttachDevices(
        std::shared_ptr<VulkanInstance> vkInstance,
        std::unique_ptr<VulkanPhysicalDevice> physicalDevice,
        std::shared_ptr<VulkanLogicalDevice> logicalDevice,
        const EngineInfo& info,
        Ref<VulkanQueue> queue,
        Ref<Renderer>* renderer,
        Ref<RenderContext>* context
        );
    // clang-format on

  private:
    WeakRef<Renderer> m_wRenderer;
    RendererFactory() = default;
};
}  // namespace fg
