#pragma once
#include "../Defines.h"
#include "Types.h"
#include "VulkanSwapchain.h"

namespace ENGINE_NAMESPACE
{
    class RenderDevice;
    class VulkanPhysicalDevice;
    class VulkanLogicalDevice;
    class VulkanDeviceContext;
    class EngineFactory
    {
        public:
            static EngineFactory* GetInstance()
            {
                static EngineFactory fac;
                return &fac;
            }
            void CreateVulkanContexts(const EngineCreateInfo& ci, RenderDevice** ppRenderDevice,
                                      VulkanDeviceContext** ppDeviceContext);
            void CreateSwapchain(RenderDevice* pRenderDevice, VulkanDeviceContext* pDevCtx,
                                 const SwapchainDescription& sDesc, GLFWwindow* window,
                                 VulkanSwapchain** ppSwapchain);

            void AttachToVulkanDevice(std::shared_ptr<VulkanInstance> instance,
                                      std::unique_ptr<VulkanPhysicalDevice> physicalDevice,
                                      std::shared_ptr<VulkanLogicalDevice> logicalDevice,
                                      const EngineCreateInfo& engineCI,
                                      const GraphicsAdapterInfo& adapterInfo,
                                      RenderDevice** ppDevice, VulkanDeviceContext** ppContext);

        private:
            EngineCreateInfo m_Ci;
    };
    GraphicsAdapterInfo GetPhysicalDeviceGraphicsAdapterInfo(const VulkanPhysicalDevice& pDevice);
}  // namespace ENGINE_NAMESPACE
