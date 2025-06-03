#pragma once

#include <memory>
#include "VulkanHeader.h"

namespace fg {

template <class T>
class VulkanObject;

template <class TResource>
class VmaAllocationWrapper;

using ShaderModuleWrapper = VulkanObject<VkShaderModule>;
using PipelineWrapper = VulkanObject<VkPipeline>;
using PipelineLayoutWrapper = VulkanObject<VkPipelineLayout>;
using CommandPoolWrapper = VulkanObject<VkCommandPool>;
using SemaphoreWrapper = VulkanObject<VkSemaphore>;
using FenceWrapper = VulkanObject<VkFence>;
using ImageWrapper = VulkanObject<VkImage>;
using BufferWrapper = VulkanObject<VkBuffer>;
using MemoryWrapper = VulkanObject<VkDeviceMemory>;
using DescriptorSetLayoutWrapper = VulkanObject<VkDescriptorSetLayout>;
using DescriptorPoolWrapper = VulkanObject<VkDescriptorPool>;
using VmaBufferWrapper = VmaAllocationWrapper<VkBuffer>;
using VmaImageWrapper = VmaAllocationWrapper<VkImage>;

class VulkanPhysicalDevice;
class Renderer;

class VulkanLogicalDevice : public std::enable_shared_from_this<VulkanLogicalDevice> {
  public:
    VulkanLogicalDevice(const VkDeviceCreateInfo& info, uint32_t queueIndex,
                        std::weak_ptr<Renderer> renderer, const VkAllocationCallbacks* allocator);

    VulkanLogicalDevice(const VulkanLogicalDevice&) = delete;
    VulkanLogicalDevice(VulkanLogicalDevice&&) = delete;
    VulkanLogicalDevice& operator=(const VulkanLogicalDevice&) = delete;
    VulkanLogicalDevice& operator=(VulkanLogicalDevice&&) = delete;

    VkQueue GetQueue() const {
      return m_Queue;
    }
    std::shared_ptr<VulkanLogicalDevice> GetPtr() {
      return shared_from_this();
    }

    std::shared_ptr<const VulkanLogicalDevice> GetPtr() const {
      return shared_from_this();
    }

    VkDevice GetHandle() const {
      return m_Device;
    }
    const VkAllocationCallbacks* GetAllocator() const {
      return m_Allocator;
    }
    void SetVMAInstance(VmaAllocator vma) {
      m_VMA = vma;
    }
    void WaitIdle() const;
    void DestroyObject(BufferWrapper&& handle) const;
    void DestroyObject(ShaderModuleWrapper&& handle) const;
    void DestroyObject(PipelineLayoutWrapper&& handle) const;
    void DestroyObject(PipelineWrapper&& handle) const;
    void DestroyObject(CommandPoolWrapper&& handle) const;
    void DestroyObject(SemaphoreWrapper&& handle) const;
    void DestroyObject(FenceWrapper&& handle) const;
    void DestroyObject(ImageWrapper&& handle) const;
    void DestroyObject(MemoryWrapper&& handle) const;
    void DestroyObject(VmaBufferWrapper&& handle) const;
    void DestroyObject(VmaImageWrapper&& handle) const;
    void DestroyObject(DescriptorSetLayoutWrapper&& handle) const;
    void DestroyObject(DescriptorPoolWrapper&& handle) const;
    void WaitFence(VkFence fence);
    void ResetFence(VkFence& fence);
    VkResult GetFenceStatus(VkFence fence);

    // clang-format off
    ShaderModuleWrapper CreateShader(const VkShaderModuleCreateInfo& info, const char* name = nullptr)const;
    PipelineLayoutWrapper CreatePipelineLayout(const VkPipelineLayoutCreateInfo& info, const char* name = nullptr)const;
    PipelineWrapper CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& info, const char* name = nullptr)const;
    CommandPoolWrapper CreateCommandPool(const VkCommandPoolCreateInfo& info, const char* name = nullptr)const;
    SemaphoreWrapper CreateVulkanSemaphore(const VkSemaphoreCreateInfo& info, const char* name = nullptr);
    FenceWrapper CreateFence(const VkFenceCreateInfo& info, const char* name = nullptr)const;
    ImageWrapper CreateImage(const VkImageCreateInfo& info, const char* name = nullptr)const;
    BufferWrapper CreateBuffer(const VkBufferCreateInfo& info, const char* name = nullptr)const;
    VmaBufferWrapper CreateVMABuffer(const VkBufferCreateInfo& info,const VmaAllocationCreateInfo& allocInfo) const;
    VmaImageWrapper CreateVMAImage(const VkImageCreateInfo& info,const VmaAllocationCreateInfo& allocInfo) const;
    DescriptorSetLayoutWrapper CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info, const char* name = nullptr)const;
    DescriptorPoolWrapper CreateDescriptorPool(const VkDescriptorPoolCreateInfo& info, const char* name = nullptr)const;
    // clang-format on

    // TODO: Should allocate from pool ?
    VkCommandBuffer AllocateCmdBuffer(const VkCommandBufferAllocateInfo& info) const;
    void FreeCmdBuffer(VkCommandPool pool, VkCommandBuffer cmd) const;
    void ResetCmdPool(VkCommandPool pool) const;

    VkMemoryRequirements GetImageMemReq(VkImage image) const;
    VkMemoryRequirements GetBufferMemReq(VkBuffer buffer) const;
    MemoryWrapper AllocateMemory(const VkMemoryAllocateInfo& memReqs) const;
    void BindImageMemory(VkImage image, VkDeviceMemory mem) const;
    void BindBufferMemory(VkBuffer buffer, VkDeviceMemory mem, uint64_t offset = 0) const;

    [[nodiscard]] void* MapBuffer(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                  VkMemoryMapFlags flags) const;
    void UnmapBuffer(VkDeviceMemory memory) const;
    void UpdateDescriptorSets(uint32_t writeCount, const VkWriteDescriptorSet* sets,
                              uint32_t dstCopyCount, const VkCopyDescriptorSet* copies);

  private:
    const VkAllocationCallbacks* m_Allocator;
    std::weak_ptr<Renderer> m_Renderer;
    VkDevice m_Device;
    VkQueue m_Queue;
    uint32_t m_QueueIndex;
    VmaAllocator m_VMA = nullptr;
};
}  // namespace fg
