#pragma once
#include "VulkanHeader.h"
#include "../Core/Assert.h"

namespace fg {

class VulkanCommandBuffer {
  public:
    VulkanCommandBuffer() noexcept;
    VulkanCommandBuffer(VulkanCommandBuffer&&) = delete;
    VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) = delete;
    VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
    ~VulkanCommandBuffer() = default;

    void FlushBarriers();
    void SetVkCommandBuffer(VkCommandBuffer cmd, VkPipelineStageFlags stageMask,
                            VkAccessFlags accessMask) {
      m_Cmd = cmd;
      m_PipelineBarrier.SupportedStagesMask = stageMask;
      m_PipelineBarrier.SupportedAccessMask = accessMask;
    }
    inline void Reset() {
      m_Barriers.clear();
      m_Cmd = VK_NULL_HANDLE;
      m_State = {};
      m_PipelineBarrier = {};
    }
    inline void BeginRendering(const VkRenderingInfoKHR& info) {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.GraphicsPipeline == VK_NULL_HANDLE);

      vkCmdBeginRendering(m_Cmd, &info);
      m_State.InsideRendering = true;
    }
    inline void SetScissor(const VkRect2D& scissor) {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      vkCmdSetScissor(m_Cmd, 0, 1, &scissor);
    }
    inline void CmdSetViewport(uint32_t firstViewport, uint32_t viewportCount,
                               const VkViewport& vp) {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      vkCmdSetViewport(m_Cmd, firstViewport, viewportCount, &vp);
    }
    inline void EndRendering() {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      vkCmdEndRenderingKHR(m_Cmd);
      m_State.InsideRendering = false;
      m_State.GraphicsPipeline = VK_NULL_HANDLE;
    }
    inline void BindGraphicsPipeline(VkPipeline pipeline) {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      vkCmdBindPipeline(m_Cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
      m_State.GraphicsPipeline = pipeline;
    }
    inline void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                     uint32_t firstInstance) {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      FOO_ASSERT(m_State.GraphicsPipeline != VK_NULL_HANDLE);
      vkCmdDraw(m_Cmd, vertexCount, instanceCount, firstVertex, firstInstance);
    }
    inline void BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                   uint32_t firstSet, uint32_t descriptorSetCount,
                                   const VkDescriptorSet* pDescriptorSets,
                                   uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      FOO_ASSERT(m_State.GraphicsPipeline != VK_NULL_HANDLE);
      vkCmdBindDescriptorSets(m_Cmd, pipelineBindPoint, layout, firstSet, descriptorSetCount,
                              pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }
    inline void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,
                                  const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      FOO_ASSERT(m_State.GraphicsPipeline != VK_NULL_HANDLE);
      vkCmdBindVertexBuffers(m_Cmd, firstBinding, bindingCount, pBuffers, pOffsets);
    }

    struct State {
        VkPipeline GraphicsPipeline = VK_NULL_HANDLE;
        uint32_t FramebufferWidth = 0, FramebufferHeight = 0;
        bool InsideRendering = false;
    };
    const State& GetState() const {
      return m_State;
    }

  private:
    struct Barrier {
        VkPipelineStageFlags MemSrcStages = 0;
        VkPipelineStageFlags MemDstStages = 0;

        VkAccessFlags MemSrcAccess = 0;
        VkAccessFlags MemDstAccess = 0;

        VkPipelineStageFlags ImageSrcStages = 0;
        VkPipelineStageFlags ImageDstStages = 0;

        VkPipelineStageFlags SupportedStagesMask = ~0u;
        VkAccessFlags SupportedAccessMask = ~0u;
    };
    State m_State;
    Barrier m_PipelineBarrier;
    VkCommandBuffer m_Cmd = VK_NULL_HANDLE;
    std::vector<VkImageMemoryBarrier> m_Barriers;
};

}  // namespace fg
