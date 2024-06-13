#include "VulkanCommandBuffer.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include "VulkanTexture.h"
#include "RenderDevice.h"
#include "VulkanBuffer.h"
#include "vulkan/vulkan_core.h"
namespace ENGINE_NAMESPACE
{
#define VERIFY_NOT_NULL(x) assert(x != VK_NULL_HANDLE)
#ifdef FOO_DEBUG
#define CMD_NOT_NULL() VERIFY_NOT_NULL(m_Cmd)
#else
#define CMD_NOT_NULL()
#endif

    VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer cmd) : m_Cmd(cmd)
    {
    }

    void VulkanCommandBuffer::End()
    {
        CMD_NOT_NULL();
        vkEndCommandBuffer(m_Cmd);
    }

    void VulkanCommandBuffer::CopyBufferToImage(VkBuffer& source, VkImage& destination,
                                                VkImageLayout layout, VkBufferImageCopy region,
                                                uint32_t regionCount)
    {
        CMD_NOT_NULL();
        vkCmdCopyBufferToImage(m_Cmd, source, destination, layout, regionCount, &region);
    }

    void VulkanCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount,
                                   uint32_t firstVertex, uint32_t firstInstance)
    {
        CMD_NOT_NULL();
        vkCmdDraw(m_Cmd, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount,
                                          uint32_t firstIndex, int32_t vertexOffset,
                                          uint32_t firstInstance)
    {
        CMD_NOT_NULL();
        vkCmdDrawIndexed(m_Cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void VulkanCommandBuffer::ClearAttachment(const VkClearAttachment& attachment,
                                              const VkClearRect& clearRect)
    {
        CMD_NOT_NULL();
        vkCmdClearAttachments(m_Cmd, 1, &attachment, 1, &clearRect);
    }
    void VulkanCommandBuffer::ClearColorImage(VkImage image, const VkClearColorValue& color,
                                              const VkImageSubresourceRange& subresource)
    {
        CMD_NOT_NULL();
        // flush barriers
        vkCmdClearColorImage(m_Cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1,
                             &subresource);
    }

    void VulkanCommandBuffer::ClearDepthStencilImage(VkImage image,
                                                     const VkClearDepthStencilValue& depthStencil,
                                                     const VkImageSubresourceRange& subresource)
    {
        CMD_NOT_NULL();
        vkCmdClearDepthStencilImage(m_Cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    &depthStencil, 1, &subresource);
    }

    void VulkanCommandBuffer::BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer,
                                              uint32_t framebufferWidth, uint32_t framebufferHeight,
                                              uint32_t clearValueCount,
                                              const VkClearValue* pClearValues)
    {
        CMD_NOT_NULL();
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = renderPass;
        renderPassInfo.framebuffer       = framebuffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {framebufferWidth, framebufferHeight};

        renderPassInfo.clearValueCount = clearValueCount;
        renderPassInfo.pClearValues    = pClearValues;

        vkCmdBeginRenderPass(m_Cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommandBuffer::EndRenderPass()
    {
        CMD_NOT_NULL();
        vkCmdEndRenderPass(m_Cmd);
    }

    void VulkanCommandBuffer::PushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                            uint32_t offset, uint32_t size, const void* pValues)
    {
        CMD_NOT_NULL();
        vkCmdPushConstants(m_Cmd, layout, stageFlags, offset, size, pValues);
    }

    void VulkanCommandBuffer::BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,
                                                const VkBuffer* pBuffers,
                                                const VkDeviceSize* pOffsets)
    {
        CMD_NOT_NULL();
        vkCmdBindVertexBuffers(m_Cmd, firstBinding, bindingCount, pBuffers, pOffsets);
    }
    void VulkanCommandBuffer::BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset,
                                              VkIndexType indexType)
    {
        CMD_NOT_NULL();
        vkCmdBindIndexBuffer(m_Cmd, buffer, offset, indexType);
    }

    void VulkanCommandBuffer::BindGraphicsPipeline(VkPipeline pipeline)
    {
        CMD_NOT_NULL();
        vkCmdBindPipeline(m_Cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    void VulkanCommandBuffer::BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipelineLayout layout, uint32_t firstSet,
                                                 uint32_t descriptorSetCount,
                                                 const VkDescriptorSet* pDescriptorSets,
                                                 uint32_t dynamicOffsetCount,
                                                 const uint32_t* pDynamicOffsets)
    {
        CMD_NOT_NULL();
        vkCmdBindDescriptorSets(m_Cmd, pipelineBindPoint, layout, firstSet, descriptorSetCount,
                                pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }

    void VulkanCommandBuffer::SetViewport(uint32_t firstViewport, uint32_t viewportCount,
                                          const VkViewport* pViewports)
    {
        CMD_NOT_NULL();
        vkCmdSetViewport(m_Cmd, firstViewport, viewportCount, pViewports);
    }

    void VulkanCommandBuffer::SetScissor(uint32_t firstScissor, uint32_t scissorCount,
                                         const VkRect2D* pScissors)
    {
        CMD_NOT_NULL();
        vkCmdSetScissor(m_Cmd, firstScissor, scissorCount, pScissors);
    }
}  // namespace ENGINE_NAMESPACE
