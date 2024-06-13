#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "EngineFactory.h"
#include "Types.h"
#include "VulkanLogicalDevice.h"
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "vulkan/vulkan_core.h"
namespace ENGINE_NAMESPACE
{
    class VulkanDeviceContext;
    class VulkanBuffer;
    struct BuffData;
    struct BuffDesc;
    class VulkanRenderPass;

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

            void CopyBuffer(VulkanBuffer& source, VulkanBuffer& destination, size_t size);

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
            size_t GetNumImmediateContexts() const { return m_pImmediateContexts.size(); }

            VulkanDeviceContext* GetImmediateContext(size_t Ctx)
            {
                return m_pImmediateContexts[Ctx];
            }
            void CreateRenderPass(const RenderPassDesc& desc, VulkanRenderPass** pRenderPass);
            void CreateFramebuffer(VulkanSwapchain* pSwapchain, FramebufferWrapper* pFramebuffer,
                                   VulkanRenderPass* pRenderPass, uint32_t count = 2);
            CommandPoolWrapper CreateCommandPool(
                HardwareQueueIndex queueFamilyIndex = HardwareQueueIndex{0});
            VkCommandBuffer AllocateCommandBuffer(VkCommandBufferAllocateInfo& info,
                                                  const char* name = "");
            void FreeCommandBuffer(VkCommandPool pool, VkCommandBuffer cmdBuffer);
            // void InitDescriptorPool();
            void InitDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo layout);
            void WaitIdle();
            void IdleGPU();
            PipelineLayoutWrapper CreatePipelineLayout(const VkPipelineLayoutCreateInfo& layout,
                                                       const char* name = "");
            PipelineWrapper CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& info,
                                                   const char* name = "");
            ShaderModuleWrapper CreateShaderModule(const VkShaderModuleCreateInfo& info,
                                                   const char* name = "");
            SamplerWrapper CreateSampler(const VkSamplerCreateInfo& info, const char* name = "");
            VkQueue GetGraphicsQueue() const;

        private:
            std::shared_ptr<VulkanInstance> m_VulkanInstance;
            std::shared_ptr<VulkanLogicalDevice> m_LogicalDevice;
            std::unique_ptr<VulkanPhysicalDevice> m_PhysicalDevice;
            std::vector<VulkanDeviceContext*> m_pImmediateContexts;
    };

}  // namespace ENGINE_NAMESPACE
