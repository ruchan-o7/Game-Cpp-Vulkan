#pragma once

// #include "Api.h"
#include "../Core/RenderDevice.h"
#include "Descriptor/DescriptorAllocator.h"
namespace FooGame
{
    class Window;
    class WindowResizeEvent;
    class RenderDevice;
    class VulkanBuffer;
    class VulkanTexture;
    class Backend
    {
        public:
            ~Backend();
            static void Init(Window& window);
            static void SwapBuffers();
            static void WaitIdle();
            static void Shutdown();
            static VkCommandBuffer BeginSingleTimeCommands();
            static void EndSingleTimeCommands(VkCommandBuffer& commandBuffer);
            static void BeginRenderpass(VkRenderPass& renderpass);

            static bool OnWindowResized(WindowResizeEvent& event);
            static uint32_t GetCurrentFrame();
            static VkExtent2D GetSwapchainExtent();
            static VkRenderPass GetRenderPass();
            static VkFramebuffer GetFramebuffer();
            static VkCommandBuffer GetCurrentCommandbuffer();
            static void BeginRenderpass();
            static VkCommandPool GetCommandPool();
            static vke::DescriptorAllocatorHandle GetAllocatorHandle();

            static RenderDevice* GetRenderDevice();

            static void TransitionImageLayout(class VulkanImage* image, VkFormat format,
                                              VkImageLayout oldLayout, VkImageLayout newLayout);
            static void CopyBufferToImage(VulkanBuffer& source, VulkanTexture& destination);

        public:
            static void CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info,
                                                  VkDescriptorSetLayout& layout);
            static void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,
                                          VkBuffer* buffer, size_t* offsets);
            static void BindIndexBuffers(VkBuffer buffer, VkDeviceSize offset,
                                         VkIndexType indexType);
            static void DrawIndexed(uint32_t indexCount, uint32_t instanceCount,
                                    uint32_t firstIndex, int32_t vertexOffset,
                                    uint32_t firstInstance);
            static void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                             uint32_t firstInstance);

            static void BindGraphicPipeline(const VkPipeline& pipeline);
            static void SetViewport(const VkViewport& viewport);
            static void SetScissor(const VkRect2D& scissor);
            static void PushConstant(const VkPipelineLayout& layout, VkShaderStageFlags stage,
                                     uint32_t offset, uint32_t size, const void* data);
            static void UpdateDescriptorSets(int32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet* pDescriptorWrites,
                                             int32_t descriptorCopyCount,
                                             const VkCopyDescriptorSet* pDescriptorCopies);
            static void BindGraphicPipelineDescriptorSets(VkPipelineLayout layout,
                                                          uint32_t firstSet,
                                                          uint32_t descriptorSetCount,
                                                          const VkDescriptorSet* pDescriptorSets,
                                                          uint32_t dynamicOffsetCount,
                                                          const uint32_t* pDynamicOffsets);

        private:
            static void BeginDrawing();
            static void Submit();
            static void InitImgui();
    };
}  // namespace FooGame
