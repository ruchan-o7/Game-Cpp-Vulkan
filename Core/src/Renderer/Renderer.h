#pragma once
#include "src/Renderer/VulkanGraphicsPipeline.h"
#include "src/Renderer/VulkanInstance.h"
#include "src/Renderer/VulkanLogicalDevice.h"
#include "src/Renderer/VulkanPhysicalDevice.h"
#include "src/Renderer/VulkanShader.h"
#include "src/Renderer/VulkanSwapchain.h"

typedef struct GLFWwindow GLFWwindow;

namespace fg {

class Renderer {
  public:
    static std::shared_ptr<Renderer> Create(GLFWwindow* window,
                                            const VkAllocationCallbacks* alloc = nullptr);
    void Destroy();

    Ref<VulkanShader> CreateShader(const ShaderDescription& desc);

    Ref<VulkanGraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineDescription& desc);

    void BeginRendering();
    void EndRendering();

    VkCommandBuffer GetCurrentCmdBuffer() const {
      return m_Cmd;
    }

  private:
    Renderer(GLFWwindow* window, const std::shared_ptr<VulkanInstance>& instance,
             std::unique_ptr<VulkanPhysicalDevice> pDevice, const VkAllocationCallbacks* alloc);

  private:
    std::unique_ptr<VulkanPhysicalDevice> m_PhysicalDevice;
    std::shared_ptr<VulkanInstance> m_Instance;
    std::shared_ptr<VulkanLogicalDevice> m_LogicalDevice;
    std::shared_ptr<VulkanSwapchain> m_Swapchain;

    const VkAllocationCallbacks* m_AllocCB;
    VkDevice m_VkDevice = VK_NULL_HANDLE;
    VkQueue m_VkQueue = VK_NULL_HANDLE;
    VkCommandBuffer m_Cmd = VK_NULL_HANDLE;
    CommandPoolWrapper m_CmdPool;

    GLFWwindow* m_Window = nullptr;
};

}  // namespace fg

// std::vector<VkExtensionProperties> AvailableInstanceExtensions;
// std::vector<VkExtensionProperties> EnabledInstanceExtensions;
