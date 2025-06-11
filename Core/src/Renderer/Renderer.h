#pragma once
#include "VulkanGraphicsPipeline.h"
#include "VulkanShader.h"
#include "VulkanSwapchain.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanCommandBuffer.h"

typedef struct GLFWwindow GLFWwindow;

namespace fg {
class RendererFactory;
struct EngineInfo;

class VulkanCommandBufferPool;
class CommandPoolManager;
class VulkanQueue;
class RenderContext;

struct CopyBufferAttr {
    VulkanBuffer* Src = nullptr;
    VulkanBuffer* Dst = nullptr;
    VkBufferCopy Regions[4];
    uint32_t RegionCount = 0;
};
struct CopyImageAttr {
    VulkanImage* Src = nullptr;
    VulkanImage* Dst = nullptr;
    VkBufferCopy Regions[4];
    uint32_t RegionCount = 0;
};
struct CopyBufferToImageAttr {
    VulkanImage* Dst = nullptr;
    VulkanBuffer* Src = nullptr;
};

class Renderer : public RefBase {
  public:
    Renderer(ReferenceCounter* counter, RendererFactory* factory, const EngineInfo& info,
             std::shared_ptr<VulkanInstance> instance,
             std::unique_ptr<VulkanPhysicalDevice> physicalDevice,
             std::shared_ptr<VulkanLogicalDevice> logicalDevice, Ref<VulkanQueue> queue);

    void CreateDeviceAndContext();
    void Destroy();

    Ref<VulkanShader> CreateShader(const ShaderDescription& desc) const;

    Ref<VulkanGraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineDescription& desc);
    Ref<VulkanImage> CreateImage(const ImageDescription& desc, const Buffer data = Buffer());
    Ref<VulkanImage> CreateImage(const ImageDescription& desc, ResourceState initialState,
                                 VkImage image);
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

    VmaAllocator GetVMA() const {
      return m_VMA;
    }
    void SetVMA(VmaAllocator vma) {
      m_VMA = vma;
    }
    std::shared_ptr<VulkanInstance> GetVkInstance() const {
      return m_Instance;
    }
    const VulkanInstance& GetVkInstance2() const {
      return *m_Instance;
    }

    void SetContext(Ref<RenderContext> ctx) {
      m_Ctx = ctx;
    }

    void TransitionImageLayout(VulkanImage* image, VkImageLayout newLayout);
    void TransitionImageState(VulkanImage& image, ResourceState oldState, ResourceState newState);
    void AllocateTransientCmdPool(CommandPoolWrapper& pool, VulkanCommandBuffer& cmd,
                                  const char* debugName = nullptr);
    void ExecuteAndDisposeTransientCmdBuff(VkCommandBuffer cmd, CommandPoolWrapper&& pool);
    void ExecuteCommandBuffer(const VkSubmitInfo& info, VkFence* fence);

    VkResult Flush(const std::function<VkResult(VulkanQueue*)>& func);
    void Flush();

    VkCommandBuffer GetCurrentCmdBuffer() const {
      return m_Cmd.Get();
    }

    VkResult Present(VkPresentInfoKHR& info);
    void WaitGPU() const;
    Ref<VulkanQueue> GetQueue() const {
      return m_Queue;
    }

  private:
    RendererFactory* m_Factory;
    std::unique_ptr<VulkanPhysicalDevice> m_PhysicalDevice;
    std::shared_ptr<VulkanInstance> m_Instance;
    std::shared_ptr<VulkanLogicalDevice> m_LogicalDevice;
    std::shared_ptr<VulkanSwapchain> m_Swapchain;

    const VkAllocationCallbacks* m_AllocCB;
    Ref<VulkanQueue> m_Queue;
    // VkQueue m_VkQueue = VK_NULL_HANDLE;
    VulkanCommandBuffer m_Cmd;
    Ref<VulkanGraphicsPipeline> m_CurrentPipeline;
    GLFWwindow* m_Window = nullptr;
    VmaAllocator m_VMA = nullptr;
    std::unique_ptr<CommandPoolManager> m_TransientCmdPoolManager;
    std::unique_ptr<VulkanCommandBufferPool> m_CmdPool;
    // VulkanImage* m_BoundImages[8];
    // uint32_t m_BoundImageCount = 0;
    WeakRef<RenderContext> m_Ctx;
};

}  // namespace fg
