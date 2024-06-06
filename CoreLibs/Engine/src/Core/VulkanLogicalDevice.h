#pragma once
#include "VulkanPhysicalDevice.h"

#include <memory>
namespace ENGINE_NAMESPACE
{
    enum class VulkanHandleTypeId : uint32_t
    {
        CommandPool,
        CommandBuffer,
        Buffer,
        BufferView,
        Image,
        ImageView,
        DeviceMemory,
        Fence,
        RenderPass,
        Pipeline,
        ShaderModule,
        PipelineLayout,
        Sampler,
        Framebuffer,
        DescriptorPool,
        DescriptorSetLayout,
        DescriptorSet,
        Semaphore,
        Queue,
        Event,
        QueryPool,
        AccelerationStructureKHR,
        PipelineCache
    };
    template <typename VulkanObjectType, VulkanHandleTypeId>
    class VulkanObjectWrapper;

#define DEFINE_VULKAN_OBJECT_WRAPPER(Type) VulkanObjectWrapper<Vk##Type, VulkanHandleTypeId::Type>
    using CommandPoolWrapper         = DEFINE_VULKAN_OBJECT_WRAPPER(CommandPool);
    using BufferWrapper              = DEFINE_VULKAN_OBJECT_WRAPPER(Buffer);
    using BufferViewWrapper          = DEFINE_VULKAN_OBJECT_WRAPPER(BufferView);
    using ImageWrapper               = DEFINE_VULKAN_OBJECT_WRAPPER(Image);
    using ImageViewWrapper           = DEFINE_VULKAN_OBJECT_WRAPPER(ImageView);
    using DeviceMemoryWrapper        = DEFINE_VULKAN_OBJECT_WRAPPER(DeviceMemory);
    using FenceWrapper               = DEFINE_VULKAN_OBJECT_WRAPPER(Fence);
    using RenderPassWrapper          = DEFINE_VULKAN_OBJECT_WRAPPER(RenderPass);
    using PipelineWrapper            = DEFINE_VULKAN_OBJECT_WRAPPER(Pipeline);
    using ShaderModuleWrapper        = DEFINE_VULKAN_OBJECT_WRAPPER(ShaderModule);
    using PipelineLayoutWrapper      = DEFINE_VULKAN_OBJECT_WRAPPER(PipelineLayout);
    using SamplerWrapper             = DEFINE_VULKAN_OBJECT_WRAPPER(Sampler);
    using FramebufferWrapper         = DEFINE_VULKAN_OBJECT_WRAPPER(Framebuffer);
    using DescriptorPoolWrapper      = DEFINE_VULKAN_OBJECT_WRAPPER(DescriptorPool);
    using DescriptorSetLayoutWrapper = DEFINE_VULKAN_OBJECT_WRAPPER(DescriptorSetLayout);
    using SemaphoreWrapper           = DEFINE_VULKAN_OBJECT_WRAPPER(Semaphore);
    using QueryPoolWrapper           = DEFINE_VULKAN_OBJECT_WRAPPER(QueryPool);
    using AccelStructWrapper         = DEFINE_VULKAN_OBJECT_WRAPPER(AccelerationStructureKHR);
    using PipelineCacheWrapper       = DEFINE_VULKAN_OBJECT_WRAPPER(PipelineCache);
#undef DEFINE_VULKAN_OBJECT_WRAPPER

    class VulkanLogicalDevice : std::enable_shared_from_this<VulkanLogicalDevice>
    {
        public:
            // clang-format off
            static std::shared_ptr<VulkanLogicalDevice> Create(
                const VulkanPhysicalDevice& physicalDevice, 
                const VkDeviceCreateInfo& deviceCi,
                const VkAllocationCallbacks* vkAllocator);
            // clang-format on

            DELETE_COPY_MOVE(VulkanLogicalDevice);
            ~VulkanLogicalDevice();

            std::shared_ptr<VulkanLogicalDevice> GetSharedPtr() { return shared_from_this(); }

            std::shared_ptr<const VulkanLogicalDevice> GetSharedPtr() const
            {
                return shared_from_this();
            }
            VkQueue GetQueue(uint32_t queueFamilyIndex, uint32_t queueIndex);

            VkDevice GetVkDevice() const { return m_VkDevice; }

            void WaitIdle() const;

            // clang-format off
             CommandPoolWrapper  CreateCommandPool   (const VkCommandPoolCreateInfo& CmdPoolCI,   const char* DebugName = "") const;
             BufferWrapper       CreateBuffer        (const VkBufferCreateInfo&      BufferCI,    const char* DebugName = "") const;
             BufferViewWrapper   CreateBufferView    (const VkBufferViewCreateInfo&  BuffViewCI,  const char* DebugName = "") const;
             ImageWrapper        CreateImage         (const VkImageCreateInfo&       ImageCI,     const char* DebugName = "") const;
             ImageViewWrapper    CreateImageView     (const VkImageViewCreateInfo&   ImageViewCI, const char* DebugName = "") const;
             SamplerWrapper      CreateSampler       (const VkSamplerCreateInfo&     SamplerCI,   const char* DebugName = "") const;
             FenceWrapper        CreateFence         (const VkFenceCreateInfo&       FenceCI,     const char* DebugName = "") const;
             RenderPassWrapper   CreateRenderPass    (const VkRenderPassCreateInfo&  RenderPassCI,const char* DebugName = "") const;
             RenderPassWrapper   CreateRenderPass    (const VkRenderPassCreateInfo2& RenderPassCI,const char* DebugName = "") const;
             DeviceMemoryWrapper AllocateDeviceMemory(const VkMemoryAllocateInfo &   AllocInfo,   const char* DebugName = "") const;

             PipelineWrapper     CreateComputePipeline   (const VkComputePipelineCreateInfo&       PipelineCI, VkPipelineCache cache, const char* DebugName = "") const;
             PipelineWrapper     CreateGraphicsPipeline  (const VkGraphicsPipelineCreateInfo&      PipelineCI, VkPipelineCache cache, const char* DebugName = "") const;
             PipelineWrapper     CreateRayTracingPipeline(const VkRayTracingPipelineCreateInfoKHR& PipelineCI, VkPipelineCache cache, const char* DebugName = "") const;

             ShaderModuleWrapper        CreateShaderModule       (const VkShaderModuleCreateInfo&        ShaderModuleCI, const char* DebugName = "") const;
             PipelineLayoutWrapper      CreatePipelineLayout     (const VkPipelineLayoutCreateInfo&      LayoutCI,       const char* DebugName = "") const;
             FramebufferWrapper         CreateFramebuffer        (const VkFramebufferCreateInfo&         FramebufferCI,  const char* DebugName = "") const;
             DescriptorPoolWrapper      CreateDescriptorPool     (const VkDescriptorPoolCreateInfo&      DescrPoolCI,    const char* DebugName = "") const;
             DescriptorSetLayoutWrapper CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& LayoutCI,       const char* DebugName = "") const;

             SemaphoreWrapper    CreateVulkanSemaphore(const VkSemaphoreCreateInfo& SemaphoreCI, const char* DebugName = "") const;
             SemaphoreWrapper    CreateTimelineSemaphore(uint64_t InitialValue, const char* DebugName = "") const;
             QueryPoolWrapper    CreateQueryPool(const VkQueryPoolCreateInfo& QueryPoolCI, const char* DebugName = "") const;
             AccelStructWrapper  CreateAccelStruct(const VkAccelerationStructureCreateInfoKHR& CI, const char* DebugName = "") const;

             VkCommandBuffer     AllocateVkCommandBuffer(const VkCommandBufferAllocateInfo& AllocInfo, const char* DebugName = "") const;
             VkDescriptorSet     AllocateVkDescriptorSet(const VkDescriptorSetAllocateInfo& AllocInfo, const char* DebugName = "") const;
             PipelineCacheWrapper CreatePipelineCache(const VkPipelineCacheCreateInfo& CI,const char* DebugName = "") const;

             void ReleaseVulkanObject(CommandPoolWrapper&&  CmdPool) const;
             void ReleaseVulkanObject(BufferWrapper&&       Buffer) const;
             void ReleaseVulkanObject(BufferViewWrapper&&   BufferView) const;
             void ReleaseVulkanObject(ImageWrapper&&        Image) const;
             void ReleaseVulkanObject(ImageViewWrapper&&    ImageView) const;
             void ReleaseVulkanObject(SamplerWrapper&&      Sampler) const;
             void ReleaseVulkanObject(FenceWrapper&&        Fence) const;
             void ReleaseVulkanObject(RenderPassWrapper&&   RenderPass) const;
             void ReleaseVulkanObject(DeviceMemoryWrapper&& Memory) const;
             void ReleaseVulkanObject(PipelineWrapper&&     Pipeline) const;
             void ReleaseVulkanObject(ShaderModuleWrapper&& ShaderModule) const;
             void ReleaseVulkanObject(PipelineLayoutWrapper&& PipelineLayout) const;
             void ReleaseVulkanObject(FramebufferWrapper&&   Framebuffer) const;
             void ReleaseVulkanObject(DescriptorPoolWrapper&& DescriptorPool) const;
             void ReleaseVulkanObject(DescriptorSetLayoutWrapper&& DescriptorSetLayout) const;
             void ReleaseVulkanObject(SemaphoreWrapper&&     Semaphore) const;
             void ReleaseVulkanObject(QueryPoolWrapper&&     QueryPool) const;
             void ReleaseVulkanObject(AccelStructWrapper&&   AccelStruct) const;
             void ReleaseVulkanObject(PipelineCacheWrapper&& PSOCache) const;

             void FreeDescriptorSet(VkDescriptorPool Pool, VkDescriptorSet Set) const;
             void FreeCommandBuffer(VkCommandPool Pool, VkCommandBuffer CmdBuffer) const;

             VkMemoryRequirements GetBufferMemoryRequirements(VkBuffer vkBuffer) const;
             VkMemoryRequirements GetImageMemoryRequirements (VkImage  vkImage ) const;
             VkDeviceAddress      GetAccelerationStructureDeviceAddress(VkAccelerationStructureKHR AS) const;
             VkResult BindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) const;
             VkResult BindImageMemory (VkImage image,   VkDeviceMemory memory, VkDeviceSize memoryOffset) const;
            // clang-format on
            VkResult MapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                               VkMemoryMapFlags flags, void** ppData) const;
            void UnmapMemory(VkDeviceMemory memory) const;

            VkResult InvalidateMappedMemoryRanges(uint32_t memoryRangeCount,
                                                  const VkMappedMemoryRange* pMemoryRanges) const;
            VkResult FlushMappedMemoryRanges(uint32_t memoryRangeCount,
                                             const VkMappedMemoryRange* pMemoryRanges) const;

            VkResult GetFenceStatus(VkFence fence) const;
            VkResult ResetFence(VkFence fence) const;
            VkResult WaitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                   uint64_t timeout) const;

            VkResult GetSemaphoreCounter(VkSemaphore TimelineSemaphore,
                                         uint64_t* pSemaphoreValue) const;
            VkResult SignalSemaphore(const VkSemaphoreSignalInfo& SignalInfo) const;
            VkResult WaitSemaphores(const VkSemaphoreWaitInfo& WaitInfo, uint64_t Timeout) const;
            void UpdateDescriptorSets(uint32_t descriptorWriteCount,
                                      const VkWriteDescriptorSet* pDescriptorWrites,
                                      uint32_t descriptorCopyCount,
                                      const VkCopyDescriptorSet* pDescriptorCopies) const;

            VkResult ResetCommandPool(VkCommandPool vkCmdPool,
                                      VkCommandPoolResetFlags flags = 0) const;

            VkResult ResetDescriptorPool(VkDescriptorPool descriptorPool,
                                         VkDescriptorPoolResetFlags flags = 0) const;

            VkResult GetQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery,
                                         uint32_t queryCount, size_t dataSize, void* pData,
                                         VkDeviceSize stride, VkQueryResultFlags flags) const
            {
                return vkGetQueryPoolResults(m_VkDevice, queryPool, firstQuery, queryCount,
                                             dataSize, pData, stride, flags);
            }

            void ResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery,
                                uint32_t queryCount) const;

            void GetAccelerationStructureBuildSizes(
                const VkAccelerationStructureBuildGeometryInfoKHR& BuildInfo,
                const uint32_t* pMaxPrimitiveCounts,
                VkAccelerationStructureBuildSizesInfoKHR& SizeInfo) const;

            VkResult GetRayTracingShaderGroupHandles(VkPipeline pipeline, uint32_t firstGroup,
                                                     uint32_t groupCount, size_t dataSize,
                                                     void* pData) const;

            VkPipelineStageFlags GetSupportedStagesMask(HardwareQueueIndex QueueFamilyIndex) const
            {
                return m_SupportedStagesMask[QueueFamilyIndex];
            }
            VkAccessFlags GetSupportedAccessMask(HardwareQueueIndex QueueFamilyIndex) const
            {
                return m_SupportedAccessMask[QueueFamilyIndex];
            }

            const VkPhysicalDeviceFeatures& GetEnabledFeatures() const { return m_EnabledFeatures; }

        private:
            VulkanLogicalDevice(const VulkanPhysicalDevice& PhysicalDevice,
                                const VkDeviceCreateInfo& DeviceCI,
                                const VkAllocationCallbacks* vkAllocator);

            template <typename VkObjectType, VulkanHandleTypeId VkTypeId,
                      typename VkCreateObjectFuncType, typename VkObjectCreateInfoType>
            VulkanObjectWrapper<VkObjectType, VkTypeId> CreateVulkanObject(
                VkCreateObjectFuncType VkCreateObject, const VkObjectCreateInfoType& CreateInfo,
                const char* DebugName, const char* ObjectType) const;

        private:
            VkDevice m_VkDevice;
            const VkAllocationCallbacks* const m_VkAllocator;
            const VkPhysicalDeviceFeatures m_EnabledFeatures;
            std::vector<VkPipelineStageFlags> m_SupportedStagesMask;
            std::vector<VkAccessFlags> m_SupportedAccessMask;
    };
}  // namespace ENGINE_NAMESPACE
