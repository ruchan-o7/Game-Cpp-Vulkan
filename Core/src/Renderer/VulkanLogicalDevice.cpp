#include "VulkanLogicalDevice.h"
#include "VulkanObject.h"
#include "VulkanPhysicalDevice.h"
#include "Renderer.h"
#include "../Core/Assert.h"
#include "../Core/Log.h"
#include "src/Renderer/VulkanHeader.h"

namespace fg {

VulkanLogicalDevice::VulkanLogicalDevice(const VkDeviceCreateInfo& info, uint32_t queueIndex,
                                         std::weak_ptr<Renderer> renderer,
                                         const VkAllocationCallbacks* allocator)
    : m_Renderer(renderer), m_Allocator(allocator), m_QueueIndex(queueIndex) {
  const auto& pDevice = m_Renderer.lock()->GetPhysicalDevice();
  VkResult res = vkCreateDevice(pDevice.GetHandle(), &info, allocator, &m_Device);
  if (res != VK_SUCCESS) {
    FOO_CORE_CRITICAL("Can not create logical device");
  }
  volkLoadDevice(m_Device);
  vkGetDeviceQueue(m_Device, queueIndex, 0, &m_Queue);
}

void VulkanLogicalDevice::WaitIdle() const {
  vkDeviceWaitIdle(m_Device);
}
ImageViewWrapper VulkanLogicalDevice::CreateImageView(const VkImageViewCreateInfo& info,
                                                      const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
  VkImageView handle = 0;
  auto res = vkCreateImageView(m_Device, &info, m_Allocator, &handle);
  return {std::move(handle), GetPtr()};
}

DescriptorPoolWrapper VulkanLogicalDevice::CreateDescriptorPool(
    const VkDescriptorPoolCreateInfo& info, const char* name) const {
  VkDescriptorPool handle = VK_NULL_HANDLE;
  auto res = vkCreateDescriptorPool(m_Device, &info, m_Allocator, &handle);
  return {std::move(handle), GetPtr()};
}

VmaBufferWrapper VulkanLogicalDevice::CreateVMABuffer(
    const VkBufferCreateInfo& info, const VmaAllocationCreateInfo& allocInfo) const {
  VkBuffer handle = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  auto res = vmaCreateBuffer(m_VMA, &info, &allocInfo, &handle, &allocation, nullptr);
  if (res != VK_SUCCESS) {
    FOO_CORE_ERROR("Can not allocate memory");
  }
  return {std::move(handle), std::move(allocation), GetPtr()};
}

VmaImageWrapper VulkanLogicalDevice::CreateVMAImage(
    const VkImageCreateInfo& info, const VmaAllocationCreateInfo& allocInfo) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
  VkImage handle = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  auto res = vmaCreateImage(m_VMA, &info, &allocInfo, &handle, &allocation, nullptr);
  if (res != VK_SUCCESS) {
    FOO_CORE_ERROR("Can not allocate memory");
  }
  return {std::move(handle), std::move(allocation), GetPtr()};
}

void VulkanLogicalDevice::ResetFence(VkFence& fence) {
  vkResetFences(m_Device, 1, &fence);
}

void VulkanLogicalDevice::WaitFence(VkFence fence) {
  vkWaitForFences(m_Device, 1, &fence, VK_TRUE, UINT64_MAX);
}

VkResult VulkanLogicalDevice::GetFenceStatus(VkFence fence) {
  return vkGetFenceStatus(m_Device, fence);
}
void VulkanLogicalDevice::UpdateDescriptorSets(uint32_t writeCount,
                                               const VkWriteDescriptorSet* sets,
                                               uint32_t dstCopyCount,
                                               const VkCopyDescriptorSet* copies) {
  vkUpdateDescriptorSets(m_Device, writeCount, sets, dstCopyCount, copies);
}

void* VulkanLogicalDevice::MapBuffer(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                     VkMemoryMapFlags flags) const {
  void* ptr = 0;
  auto res = vkMapMemory(m_Device, memory, offset, size, flags, &ptr);
  FOO_ASSERT(res == VK_SUCCESS);
  return ptr;
}

void VulkanLogicalDevice::UnmapBuffer(VkDeviceMemory memory) const {
  FOO_ASSERT(memory != VK_NULL_HANDLE);
  vkUnmapMemory(m_Device, memory);
}

VkMemoryRequirements VulkanLogicalDevice::GetImageMemReq(VkImage image) const {
  VkMemoryRequirements memReqs {};
  vkGetImageMemoryRequirements(m_Device, image, &memReqs);
  return memReqs;
}
VkMemoryRequirements VulkanLogicalDevice::GetBufferMemReq(VkBuffer buffer) const {
  VkMemoryRequirements memReqs {};
  vkGetBufferMemoryRequirements(m_Device, buffer, &memReqs);
  return memReqs;
}

MemoryWrapper VulkanLogicalDevice::AllocateMemory(const VkMemoryAllocateInfo& info) const {
  VkDeviceMemory mem = VK_NULL_HANDLE;
  vkAllocateMemory(m_Device, &info, m_Allocator, &mem);
  return MemoryWrapper(std::move(mem), GetPtr());
}

void VulkanLogicalDevice::BindImageMemory(VkImage image, VkDeviceMemory mem) const {
  vkBindImageMemory(m_Device, image, mem, 0);
}
void VulkanLogicalDevice::BindBufferMemory(VkBuffer buffer, VkDeviceMemory mem,
                                           uint64_t offset) const {
  vkBindBufferMemory(m_Device, buffer, mem, offset);
}

ShaderModuleWrapper VulkanLogicalDevice::CreateShader(const VkShaderModuleCreateInfo& info,
                                                      const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
  VkShaderModule module;
  auto res = vkCreateShaderModule(m_Device, &info, m_Allocator, &module);
  return ShaderModuleWrapper(std::move(module), GetPtr());
}
PipelineLayoutWrapper VulkanLogicalDevice::CreatePipelineLayout(
    const VkPipelineLayoutCreateInfo& info, const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
  VkPipelineLayout layout = VK_NULL_HANDLE;
  auto res = vkCreatePipelineLayout(m_Device, &info, m_Allocator, &layout);
  return PipelineLayoutWrapper(std::move(layout), GetPtr());
}

PipelineWrapper VulkanLogicalDevice::CreateGraphicsPipeline(
    const VkGraphicsPipelineCreateInfo& info, const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
  VkPipeline handle = VK_NULL_HANDLE;
  auto res = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &info, m_Allocator, &handle);
  return PipelineWrapper(std::move(handle), GetPtr());
}
DescriptorSetLayoutWrapper VulkanLogicalDevice::CreateDescriptorSetLayout(
    const VkDescriptorSetLayoutCreateInfo& info, const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
  VkDescriptorSetLayout handle = VK_NULL_HANDLE;
  auto res = vkCreateDescriptorSetLayout(m_Device, &info, m_Allocator, &handle);
  return DescriptorSetLayoutWrapper(std::move(handle), GetPtr());
}

CommandPoolWrapper VulkanLogicalDevice::CreateCommandPool(const VkCommandPoolCreateInfo& info,
                                                          const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
  VkCommandPool handle = VK_NULL_HANDLE;
  auto res = vkCreateCommandPool(m_Device, &info, m_Allocator, &handle);
  return CommandPoolWrapper(std::move(handle), GetPtr());
}

SamplerWrapper VulkanLogicalDevice::CreateSampler(const VkSamplerCreateInfo& info,
                                                  const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
  VkSampler sampler = 0;
  auto res = vkCreateSampler(m_Device, &info, m_Allocator, &sampler);
  return {std::move(sampler), GetPtr()};
}

SemaphoreWrapper VulkanLogicalDevice::CreateVulkanSemaphore(const VkSemaphoreCreateInfo& info,
                                                            const char* name) {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
  VkSemaphore handle;
  auto res = vkCreateSemaphore(m_Device, &info, m_Allocator, &handle);
  return SemaphoreWrapper(std::move(handle), GetPtr());
}
FenceWrapper VulkanLogicalDevice::CreateFence(const VkFenceCreateInfo& info,
                                              const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
  VkFence handle;
  auto res = vkCreateFence(m_Device, &info, m_Allocator, &handle);
  return FenceWrapper(std::move(handle), GetPtr());
}

ImageWrapper VulkanLogicalDevice::CreateImage(const VkImageCreateInfo& info,
                                              const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
  VkImage handle;
  auto res = vkCreateImage(m_Device, &info, m_Allocator, &handle);
  return ImageWrapper(std::move(handle), GetPtr());
}

BufferWrapper VulkanLogicalDevice::CreateBuffer(const VkBufferCreateInfo& info,
                                                const char* name) const {
  FOO_ASSERT(info.sType == VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
  VkBuffer handle;
  auto res = vkCreateBuffer(m_Device, &info, m_Allocator, &handle);
  return BufferWrapper(std::move(handle), GetPtr());
}

VkCommandBuffer VulkanLogicalDevice::AllocateCmdBuffer(
    const VkCommandBufferAllocateInfo& info) const {
  VkCommandBuffer handle = VK_NULL_HANDLE;
  auto res = vkAllocateCommandBuffers(m_Device, &info, &handle);
  return handle;
}

void VulkanLogicalDevice::ResetCmdPool(VkCommandPool pool) const {
  vkResetCommandPool(m_Device, pool, 0);
}

void VulkanLogicalDevice::FreeCmdBuffer(VkCommandPool pool, VkCommandBuffer cmd) const {
  vkFreeCommandBuffers(m_Device, pool, 1, &cmd);
}

void VulkanLogicalDevice::DestroyObject(BufferWrapper&& handle) const {
  vkDestroyBuffer(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}

void VulkanLogicalDevice::DestroyObject(SamplerWrapper&& handle) const {
  vkDestroySampler(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}

void VulkanLogicalDevice::DestroyObject(DescriptorSetLayoutWrapper&& handle) const {
  vkDestroyDescriptorSetLayout(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}

void VulkanLogicalDevice::DestroyObject(ShaderModuleWrapper&& module) const {
  vkDestroyShaderModule(m_Device, module, m_Allocator);
  module.m_VulkanObject = VK_NULL_HANDLE;
}
void VulkanLogicalDevice::DestroyObject(ImageWrapper&& module) const {
  vkDestroyImage(m_Device, module, m_Allocator);
  module.m_VulkanObject = VK_NULL_HANDLE;
}

void VulkanLogicalDevice::DestroyObject(PipelineLayoutWrapper&& handle) const {
  vkDestroyPipelineLayout(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}
void VulkanLogicalDevice::DestroyObject(PipelineWrapper&& handle) const {
  vkDestroyPipeline(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}
void VulkanLogicalDevice::DestroyObject(MemoryWrapper&& handle) const {
  vkFreeMemory(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}

void VulkanLogicalDevice::DestroyObject(VmaImageWrapper&& handle) const {
  vmaDestroyImage(m_VMA, handle.m_VulkanObject, handle.m_VmaAllocation);
  handle.m_VulkanObject = VK_NULL_HANDLE;
  handle.m_VmaAllocation = nullptr;
}

void VulkanLogicalDevice::DestroyObject(ImageViewWrapper&& handle) const {
  vkDestroyImageView(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}

void VulkanLogicalDevice::DestroyObject(VmaBufferWrapper&& handle) const {
  vmaDestroyBuffer(m_VMA, handle.m_VulkanObject, handle.m_VmaAllocation);
  handle.m_VulkanObject = VK_NULL_HANDLE;
  handle.m_VmaAllocation = nullptr;
}
void VulkanLogicalDevice::DestroyObject(DescriptorPoolWrapper&& handle) const {
  vkDestroyDescriptorPool(m_Device, handle.m_VulkanObject, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}

void VulkanLogicalDevice::DestroyObject(CommandPoolWrapper&& handle) const {
  vkDestroyCommandPool(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}
void VulkanLogicalDevice::DestroyObject(SemaphoreWrapper&& handle) const {
  vkDestroySemaphore(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}
void VulkanLogicalDevice::DestroyObject(FenceWrapper&& handle) const {
  vkDestroyFence(m_Device, handle, m_Allocator);
  handle.m_VulkanObject = VK_NULL_HANDLE;
}
}  // namespace fg
