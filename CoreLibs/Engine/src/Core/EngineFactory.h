#pragma once
#include "../Defines.h"
#include "Types.h"
#include "VulkanSwapchain.h"

namespace ENGINE_NAMESPACE
{
    class RenderDevice;
    class VulkanPhysicalDevice;
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

        private:
            EngineCreateInfo m_Ci;
    };
    GraphicsAdapterInfo GetPhysicalDeviceGraphicsAdapterInfo(const VulkanPhysicalDevice& pDevice);
}  // namespace ENGINE_NAMESPACE
