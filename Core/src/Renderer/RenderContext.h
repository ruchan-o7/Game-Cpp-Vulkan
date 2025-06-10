#pragma once
#include "../Core/Ref.h"
#include "VulkanHeader.h"
#include "VulkanCommandBuffer.h"
#include "Misc.h"

namespace fg {

class VulkanGraphicsPipeline;
class VulkanSwapchain;
class VulkanImage;
class VulkanBuffer;
class VulkanImageView;
class Renderer;
class VulkanCommandBufferPool;
struct EngineInfo;
class VulkanDescriptorSet;

struct RenderTargetAttr {
    uint32_t RenderTargetCount = 0;
    Ref<VulkanImageView> ppRenderTargets[8];
    Ref<VulkanImageView> DepthStencil = nullptr;
};
struct VertexBufferBindingAttr {
    uint32_t FirstBinding = 0;
    uint32_t BindingCount = 0;
    VulkanBuffer** ppBuffers = nullptr;
    uint64_t* Offsets = 0;
};

struct RenderContextInfo {
    std::string Name;
    uint8_t Id;
    VkQueueFlags Flags;
};

struct DrawAttributes {
    uint32_t VertexCount = 0;
    uint32_t InstanceCount = 0;
    uint32_t FirstVertex = 0;
    uint32_t FirstInstance = 0;
};

class RenderContext : public RefBase {
  public:
    RenderContext(Ref<Renderer> renderer, const RenderContextInfo& info);
    ~RenderContext() = default;

    RenderContext(RenderContext&&) = delete;
    RenderContext(const RenderContext&) = delete;
    RenderContext& operator=(RenderContext&&) = delete;
    RenderContext& operator=(const RenderContext&) = delete;

    void SetRenderTargets(const RenderTargetAttr& attr);
    void EndRendering();
    void BindPipeline(Ref<VulkanGraphicsPipeline> pipeline);
    void BindVertexBuffers(const VertexBufferBindingAttr& attr);
    void BindDescriptorSets(VulkanDescriptorSet* sets[], uint32_t setCount);
    void Draw(const DrawAttributes& attr);
    void Flush();

    void TransitionImageLayout(VulkanImage* image, VkImageLayout newLayout);
    void TransitionImageState(VulkanImage& image, ResourceState oldState, ResourceState newState);

  private:
    void PrepareCmdBuffer();

  private:
    std::vector<Ref<VulkanImage>> m_BoundImages;
    Ref<VulkanSwapchain> m_Swapchain;

    Ref<Renderer> m_Renderer;
    Ref<VulkanImageView> m_BoundDepthStencil;
    Ref<VulkanGraphicsPipeline> m_BoundPipeline;
    std::unique_ptr<VulkanCommandBufferPool> m_CmdPool;
    VulkanCommandBuffer m_Cmd;

    uint64_t m_FrameNumber = 0;
    RenderContextInfo m_Info;
};

}  // namespace fg
