#include "VulkanDeviceContext.h"
#include "RenderDevice.h"
namespace ENGINE_NAMESPACE
{
    VulkanDeviceContext::VulkanDeviceContext(RenderDevice* pDevice,
                                             const EngineCreateInfo& engineCI,
                                             const DeviceContextDesc& desc)
        : m_pRenderDevice(pDevice), m_DevDesc(desc), m_EngineCI(engineCI)
    {
    }

    void VulkanDeviceContext::Draw()
    {
    }
    void VulkanDeviceContext::DrawIndexed()
    {
    }
    void VulkanDeviceContext::DrawMesh()
    {
    }
    void VulkanDeviceContext::IdleGPU()
    {
        m_pRenderDevice->WaitIdle();
    }

}  // namespace ENGINE_NAMESPACE
