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
      // m_PipelineBarrier.SupportedStagesMask = stageMask;
      // m_PipelineBarrier.SupportedAccessMask = accessMask;
    }
    VkCommandBuffer Get() const {
      return m_Cmd;
    }
    inline void Reset() {
      m_ImageBarriers.clear();
      m_Cmd = VK_NULL_HANDLE;
      m_State = {};
      m_PipelineBarrier = {};
    }
    inline void EndCommandBuffer() const {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      vkEndCommandBuffer(m_Cmd);
    }
    inline void BeginRendering(const VkRenderingInfoKHR& info) {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.GraphicsPipeline == VK_NULL_HANDLE);
      FlushBarriers();
      vkCmdBeginRendering(m_Cmd, &info);
      m_State.InsideRendering = true;
    }
    inline void SetScissor(const VkRect2D& scissor) const {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      vkCmdSetScissor(m_Cmd, 0, 1, &scissor);
    }
    inline void CmdSetViewport(uint32_t firstViewport, uint32_t viewportCount,
                               const VkViewport& vp) const {
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
                     uint32_t firstInstance) const {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      FOO_ASSERT(m_State.GraphicsPipeline != VK_NULL_HANDLE);
      vkCmdDraw(m_Cmd, vertexCount, instanceCount, firstVertex, firstInstance);
    }
    inline void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, VkBuffer* buffers,
                                  VkDeviceSize* offsets) const {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      vkCmdBindVertexBuffers(m_Cmd, firstBinding, bindingCount, buffers, offsets);
    }
    inline void BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                   uint32_t firstSet, uint32_t descriptorSetCount,
                                   const VkDescriptorSet* pDescriptorSets,
                                   uint32_t dynamicOffsetCount,
                                   const uint32_t* pDynamicOffsets) const {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      FOO_ASSERT(m_State.GraphicsPipeline != VK_NULL_HANDLE);
      vkCmdBindDescriptorSets(m_Cmd, pipelineBindPoint, layout, firstSet, descriptorSetCount,
                              pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }
    inline void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,
                                  const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      FOO_ASSERT(m_State.InsideRendering);
      FOO_ASSERT(m_State.GraphicsPipeline != VK_NULL_HANDLE);
      vkCmdBindVertexBuffers(m_Cmd, firstBinding, bindingCount, pBuffers, pOffsets);
    }

    void TransitionImageLayout(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout,
                               const VkImageSubresourceRange& SubresRange,
                               VkPipelineStageFlags SrcStages, VkPipelineStageFlags DstStages);

    void InsertMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                             VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages) {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      if (m_State.InsideRendering) {
        EndRendering();
      }
      m_PipelineBarrier.MemSrcStages |= srcStages;
      m_PipelineBarrier.MemDstStages |= dstStages;

      m_PipelineBarrier.MemSrcAccess |= srcAccessMask;
      m_PipelineBarrier.MemDstAccess |= dstAccessMask;
    }
    inline void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage,
                                  VkImageLayout dstImageLayout, uint32_t regionCount,
                                  const VkBufferImageCopy* pRegions) {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      if (m_State.InsideRendering) {
        EndRendering();
      }
      FlushBarriers();
      vkCmdCopyBufferToImage(m_Cmd, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                    const VkBufferCopy* pRegions) {
      FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);
      if (m_State.InsideRendering) {
        EndRendering();
      }
      FlushBarriers();
      vkCmdCopyBuffer(m_Cmd, srcBuffer, dstBuffer, regionCount, pRegions);
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

        // VkPipelineStageFlags SupportedStagesMask = ~0u;
        // VkAccessFlags SupportedAccessMask = ~0u;
    };
    State m_State;
    Barrier m_PipelineBarrier;
    VkCommandBuffer m_Cmd = VK_NULL_HANDLE;
    std::vector<VkImageMemoryBarrier> m_ImageBarriers;
};

}  // namespace fg
