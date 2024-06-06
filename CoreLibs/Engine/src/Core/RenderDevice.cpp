#include "RenderDevice.h"
#include <cassert>
namespace ENGINE_NAMESPACE
{

    RenderDevice::RenderDevice(EngineFactory* engineFactory, const EngineCreateInfo& engineCI,
                               const GraphicsAdapterInfo& adapterInfo,
                               std::shared_ptr<VulkanInstance> VulkanInstance,
                               std::shared_ptr<VulkanLogicalDevice> LogicalDevice,
                               std::unique_ptr<VulkanPhysicalDevice> PhysicalDevice)
        : m_VulkanInstance(VulkanInstance),
          m_LogicalDevice(LogicalDevice),
          m_PhysicalDevice(std::move(PhysicalDevice))
    {
    }

    void RenderDevice::WaitIdle()
    {
        m_LogicalDevice->WaitIdle();
    }

    void RenderDevice::SetImmediateContext(uint32_t index, VulkanDeviceContext* ctx)
    {
        // todo
    }
}  // namespace ENGINE_NAMESPACE
