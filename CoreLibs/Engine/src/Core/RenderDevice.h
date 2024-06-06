#pragma once
#include <memory>
#include <vector>
#include "EngineFactory.h"
#include "Types.h"
#include "VulkanLogicalDevice.h"
#include "VulkanDeviceContext.h"
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
namespace ENGINE_NAMESPACE
{
    class VulkanBuffer;
    struct BuffData;
    struct BuffDesc;
    class RenderDevice
    {
        public:
            RenderDevice(EngineFactory* engineFactory, const EngineCreateInfo& engineCI,
                         const GraphicsAdapterInfo& adapterInfo,
                         std::shared_ptr<VulkanInstance> VulkanInstance,
                         std::shared_ptr<VulkanLogicalDevice> LogicalDevice,
                         std::unique_ptr<VulkanPhysicalDevice> PhysicalDevice);
            ~RenderDevice();

        public:
            void CreateBuffer(const BuffDesc& buffDesc, const BuffData& pBuffData,
                              VulkanBuffer** ppBuffer);

            VkDevice GetVkDevice() const { return m_LogicalDevice->GetVkDevice(); }
            const std::shared_ptr<VulkanLogicalDevice> GetLogicalDevice() const
            {
                return m_LogicalDevice;
            }
            const VulkanPhysicalDevice* GetPhysicalDevice() const { return m_PhysicalDevice.get(); }
            VkPhysicalDevice GetVkPhysicalDevice() const
            {
                return m_PhysicalDevice->GetVkDeviceHandle();
            }
            VkInstance GetVkInstance() const { return m_VulkanInstance->GetVkInstance(); }
            size_t GetNumImmediateContexts() const { return m_wpImmediateContexts.size(); }
            size_t GetNumDeferredContexts() const { return m_wpDeferredContexts.size(); }

            std::shared_ptr<VulkanDeviceContext> GetImmediateContext(size_t Ctx)
            {
                return m_wpImmediateContexts[Ctx].lock();
            }
            std::shared_ptr<VulkanDeviceContext> GetDeferredContext(size_t Ctx)
            {
                return m_wpDeferredContexts[Ctx].lock();
            }
            void WaitIdle();

        private:
            std::shared_ptr<VulkanInstance> m_VulkanInstance;
            std::shared_ptr<VulkanLogicalDevice> m_LogicalDevice;
            std::unique_ptr<VulkanPhysicalDevice> m_PhysicalDevice;
            std::vector<std::weak_ptr<VulkanDeviceContext>> m_wpImmediateContexts;
            std::vector<std::weak_ptr<VulkanDeviceContext>> m_wpDeferredContexts;
    };

}  // namespace ENGINE_NAMESPACE
