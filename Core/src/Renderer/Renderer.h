#pragma once
#include "../Renderer/VulkanGraphicsPipeline.h"
#include "../Renderer/VulkanShader.h"
#include "../Renderer/VulkanSwapchain.h"

typedef struct GLFWwindow GLFWwindow;

namespace fg {

struct DrawAttributes {
    uint32_t VertexCount = 0;
    uint32_t InstanceCount = 0;
    uint32_t FirstVertex = 0;
    uint32_t FirstInstance = 0;
};

class Renderer {
  public:
    static std::shared_ptr<Renderer> Create(GLFWwindow* window,
                                            const VkAllocationCallbacks* alloc = nullptr);
    void Destroy();

    Ref<VulkanShader> CreateShader(const ShaderDescription& desc) const;

    Ref<VulkanGraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineDescription& desc);

    std::shared_ptr<VulkanSwapchain> GetSwapchain() const {
      return m_Swapchain;
    }
    void BeginRendering();
    void BindPipeline(const Ref<VulkanGraphicsPipeline>& pipeline);
    void Draw(const DrawAttributes& attribs);
    void EndRendering();

    VkResult Flush(const std::function<VkResult(VkQueue, VkCommandBuffer)>& func);

    VkCommandBuffer GetCurrentCmdBuffer() const {
      return m_Cmd;
    }

    void Present(VkPresentInfoKHR& info);
    void WaitGPU() const;

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
    Ref<VulkanGraphicsPipeline> m_CurrentPipeline;
    GLFWwindow* m_Window = nullptr;
};

}  // namespace fg

// std::vector<VkExtensionProperties> AvailableInstanceExtensions;
// std::vector<VkExtensionProperties> EnabledInstanceExtensions;
