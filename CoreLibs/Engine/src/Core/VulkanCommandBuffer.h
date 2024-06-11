#pragma once
#include <cstdint>
#include "VulkanLogicalDevice.h"
namespace ENGINE_NAMESPACE
{
    class VulkanBuffer;
    class VulkanTexture;
    class VulkanImage;
    class VulkanCommandBuffer
    {
            DELETE_COPY_MOVE(VulkanCommandBuffer);

        public:
            VulkanCommandBuffer(VkCommandBuffer cmd);
            VkCommandBuffer& operator()() { return m_Cmd; }

        public:
            void CopyBufferToImage(VkBuffer& source, VkImage& destination, VkImageLayout layout,
                                   VkBufferImageCopy region, uint32_t regionCount = 1);
            void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                      uint32_t firstInstance);
            void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                             int32_t vertexOffset, uint32_t firstInstance);
            void ClearAttachment(const VkClearAttachment& attachment, const VkClearRect& clearRect);
            void ClearColorImage(VkImage image, const VkClearColorValue& color,
                                 const VkImageSubresourceRange& subresource);
            void ClearDepthStencilImage(VkImage image, const VkClearDepthStencilValue& depthStencil,
                                        const VkImageSubresourceRange& subresource);
            void BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer,
                                 uint32_t framebufferWidth, uint32_t framebufferHeight,
                                 uint32_t clearValueCount         = 0,
                                 const VkClearValue* pClearValues = nullptr);
            void EndRenderPass();
            void PushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                               uint32_t offset, uint32_t size, const void* pValues);
            void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,
                                   const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
            void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset,
                                 VkIndexType indexType = VK_INDEX_TYPE_UINT32);
            void BindGraphicsPipeline(VkPipeline pipeline);
            void BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                    uint32_t firstSet, uint32_t descriptorSetCount,
                                    const VkDescriptorSet* pDescriptorSets,
                                    uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
            void SetViewport(uint32_t firstViewport, uint32_t viewportCount,
                             const VkViewport* pViewports);
            void SetScissor(uint32_t firstScissor, uint32_t scissorCount,
                            const VkRect2D* pScissors);
            void End();

        private:
            VkCommandBuffer m_Cmd;
    };

}  // namespace ENGINE_NAMESPACE
