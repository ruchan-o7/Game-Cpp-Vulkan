#include "VulkanBuffer.h"
#include "Renderer.h"
#include "VulkanPhysicalDevice.h"
#include "../Core/Assert.h"
#include "../Core/Log.h"

namespace fg {

VkBufferUsageFlags ToVk(BufferUsage usage) {
  switch (usage) {
    case BufferUsage::Vertex:
      return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    case BufferUsage::Index:
      return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    case BufferUsage::Uniform:
      return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    case BufferUsage::Staging:
      return VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    case BufferUsage::None:
      return 0;
  }
}
VkMemoryPropertyFlags GetMemFlag(BufferUsage usage) {
  switch (usage) {
    case BufferUsage::Vertex:
    case BufferUsage::Index:
      return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    case BufferUsage::Staging:
    case BufferUsage::Uniform:
      return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    case BufferUsage::None:
      FOO_CORE_ERROR("No buffer usage specified while creating buffer");
      return 0;
  }

  FOO_ASSERT("Did not implemented");
  return 0;
}

VulkanBuffer::VulkanBuffer(const BufferDescription& desc, std::weak_ptr<Renderer> renderer,
                           const Buffer data)
    : m_Renderer(renderer), m_Desc(desc) {
  VkBufferCreateInfo info {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  info.size = m_Desc.Size;
  info.usage = ToVk(m_Desc.Usage);
  info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  auto r = m_Renderer.lock();
  auto device = r->GetLogicalDevice();
  m_Handle = device->CreateBuffer(info, m_Desc.Name);

  auto memRequirements = device->GetBufferMemReq(m_Handle);
  VkMemoryPropertyFlags flags = GetMemFlag(m_Desc.Usage);

  auto memTypeIndex =
      r->GetPhysicalDevice().FindMemTypeIndex(memRequirements.memoryTypeBits, flags);

  VkMemoryAllocateInfo allocInfo {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = memTypeIndex;

  m_Memory = device->AllocateMemory(allocInfo);
  device->BindBufferMemory(m_Handle, m_Memory, 0);

  if (data) {
    if (m_Desc.Usage == BufferUsage::Staging || m_Desc.Usage == BufferUsage::Uniform) {
      SetData(data);
    } else {
      BufferDescription stageDesc;
      stageDesc.Size = desc.Size;
      stageDesc.Usage = BufferUsage::Staging;
      stageDesc.Name = desc.Name;

      auto stageBuffer = r->CreateBuffer(stageDesc);
      stageBuffer->SetData(data);
      VkBufferCopy region[] = {
          {0, 0, m_Desc.Size}
      };
      stageBuffer->CopyTo(this, region, 1);
    }
  }
}

void VulkanBuffer::SetData(Buffer buffer) {
  Map();
  std::memcpy(m_MapPtr, buffer.Data, buffer.Size);
  Unmap();
}

void VulkanBuffer::Map(VkMemoryMapFlags flags) {
  FOO_ASSERT(m_MapPtr == nullptr, "Buffer did not unmapped after mapped");
  FOO_ASSERT(m_Handle);
  FOO_ASSERT(m_Memory);
  FOO_ASSERT(!m_Renderer.expired());
  if (auto renderer = m_Renderer.lock()) {
    auto device = renderer->GetLogicalDevice();
    m_MapPtr = device->MapBuffer(m_Memory, 0, m_Desc.Size, flags);
  }
}

void VulkanBuffer::Unmap() {
  FOO_ASSERT(m_Handle);
  FOO_ASSERT(m_Memory);
  FOO_ASSERT(!m_Renderer.expired());
  if (auto renderer = m_Renderer.lock()) {
    auto device = renderer->GetLogicalDevice();
    device->UnmapBuffer(m_Memory);
    m_MapPtr = 0;
  }
}

void VulkanBuffer::CopyTo(VulkanBuffer* destination, VkBufferCopy* regions, uint32_t regionCount) {
  auto r = m_Renderer.lock();
  auto cmd = r->GetTransientCmdBuffer();
  vkCmdCopyBuffer(cmd, m_Handle, destination->GetVkBuffer(), regionCount, regions);
  r->SubmitTransientCommandBuffer(cmd);
}

VulkanBuffer::~VulkanBuffer() {
  if (m_MapPtr) {
    Unmap();
  }
}

}  // namespace fg
