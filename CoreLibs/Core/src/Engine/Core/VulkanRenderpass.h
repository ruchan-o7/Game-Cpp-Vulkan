#pragma once
#include "../Defines.h"
#include "VulkanLogicalDevice.h"
#include "Types.h"
#include "Utils/VulkanObjectWrapper.h"
#include <vulkan/vulkan.h>
namespace ENGINE_NAMESPACE
{
    class RenderDevice;
    class VulkanRenderPass
    {
        public:
            VulkanRenderPass(RenderDevice* pRenderDevice, const RenderPassDesc& desc);
            VkRenderPass GetRenderPass() const { return m_RenderPass; }

        private:
            void Create();

        private:
            RenderPassWrapper m_RenderPass;
            RenderDevice* m_pRenderDevice;
            RenderPassDesc m_Desc;
    };

}  // namespace ENGINE_NAMESPACE
