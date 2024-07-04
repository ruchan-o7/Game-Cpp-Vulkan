#pragma once
#include <cassert>
#include "Types.h"
#include "vulkan/vulkan_core.h"
namespace vke
{
    class DescriptorAllocatorPool;
}
namespace ENGINE_NAMESPACE
{
    class RenderDevice;
    class VulkanBuffer;
    class VulkanDeviceContext
    {
        public:
            VulkanDeviceContext(RenderDevice* pDevice, const EngineCreateInfo& engineCI,
                                const DeviceContextDesc& desc = {});
            ~VulkanDeviceContext();

        public:
            void TransitionImageLayout() { assert(0 && "Not implemented"); }
            void Flush() { assert(0 && "Not implemented"); }
            void FinishFrame() { assert(0 && "Not implemented"); }
            void AddWaitSemaphore(VkSemaphore* semaphore, VkPipelineStageFlags stageBits) {}
            // Draw
            void Draw();
            void DrawIndexed();
            void DrawMesh();

            void IdleGPU();
            RenderDevice* GetVkDevice() const { return m_pRenderDevice; }

        private:
        private:
            RenderDevice* m_pRenderDevice;
            DeviceContextDesc m_DevDesc;
            EngineCreateInfo m_EngineCI;
            std::unique_ptr<vke::DescriptorAllocatorPool> m_DescriptorAllocator;

            std::vector<VkSemaphore> m_WaitSemaphores;  // image available
            std::vector<VkSemaphore> m_RenderFinishedSemaphores;
            VkCommandPool m_CmdPool         = nullptr;
            VkFramebuffer m_FrameBuffer     = nullptr;
            VkRenderPass m_VkRenderPass     = nullptr;
            VkCommandBuffer m_CommandBuffer = nullptr;

            std::vector<std::unique_ptr<VulkanBuffer>> m_UniformBuffers;
            VulkanBuffer* m_CurrentUniformBuffer;
    };
}  // namespace ENGINE_NAMESPACE
