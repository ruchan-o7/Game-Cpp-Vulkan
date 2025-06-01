#pragma once
#include "../Renderer/VulkanGraphicsPipeline.h"
#include "../Renderer/VulkanShader.h"
#include "../Renderer/VulkanSwapchain.h"
#include "src/Renderer/VulkanBuffer.h"
#include "src/Renderer/VulkanImage.h"

typedef struct GLFWwindow GLFWwindow;

namespace fg {

struct DrawAttributes {
    uint32_t VertexCount = 0;
    uint32_t InstanceCount = 0;
    uint32_t FirstVertex = 0;
    uint32_t FirstInstance = 0;
};

class Renderer : public std::enable_shared_from_this<Renderer> {
  public:
    static std::shared_ptr<Renderer> Create(GLFWwindow* window,
                                            const VkAllocationCallbacks* alloc = nullptr);
    void CreateDeviceAndSwapchain();
    void Destroy();

    std::shared_ptr<Renderer> GetPtr() {
      return shared_from_this();
    }

    std::shared_ptr<const Renderer> GetPtr() const {
      return shared_from_this();
    }

    Ref<VulkanShader> CreateShader(const ShaderDescription& desc) const;

    Ref<VulkanGraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineDescription& desc);
    Ref<VulkanImage> CreateImage(const ImageDescription& desc, VkImage handle);
    Ref<VulkanBuffer> CreateBuffer(const BufferDescription& desc, Buffer bufferData = Buffer());

    std::shared_ptr<VulkanSwapchain> GetSwapchain() const {
      return m_Swapchain;
    }

    std::shared_ptr<VulkanLogicalDevice> GetLogicalDevice() const {
      return m_LogicalDevice;
    }

    const VulkanPhysicalDevice& GetPhysicalDevice() {
      return *m_PhysicalDevice;
    }

    VkCommandBuffer GetTransientCmdBuffer();
    void SubmitTransientCommandBuffer(VkCommandBuffer cmd);

    void BeginRendering();
    void BindPipeline(const Ref<VulkanGraphicsPipeline>& pipeline);
    void Draw(const DrawAttributes& attribs);
    void EndRendering();
    void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, VulkanBuffer** buffers,
                           VkDeviceSize* offsets) const;

    VkResult Flush(const std::function<VkResult(VkQueue, VkCommandBuffer)>& func);

    VkCommandBuffer GetCurrentCmdBuffer() const {
      return m_Cmd;
    }

    VkResult Present(VkPresentInfoKHR& info);
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
