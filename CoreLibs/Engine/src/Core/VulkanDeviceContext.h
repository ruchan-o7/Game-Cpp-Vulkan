#pragma once
#include <cassert>
#include "Types.h"
namespace ENGINE_NAMESPACE
{
    class RenderDevice;
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
            RenderDevice* m_pRenderDevice;
            DeviceContextDesc m_DevDesc;
            EngineCreateInfo m_EngineCI;
    };
}  // namespace ENGINE_NAMESPACE
