#include "VulkanBuffer.h"
#include "Renderer.h"
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

VmaMemoryUsage GetVMAUsage(BufferUsage usage) {
  switch (usage) {
    case BufferUsage::Vertex:
    case BufferUsage::Index:
      return VMA_MEMORY_USAGE_GPU_ONLY;
    case BufferUsage::Uniform:
    case BufferUsage::Staging:
    case BufferUsage::None:
      return VMA_MEMORY_USAGE_AUTO;
      break;
  }
}

VulkanBuffer::VulkanBuffer(const BufferDescription& desc, const Renderer* renderer,
                           const Buffer data)
    : m_Desc(desc) {
  m_Allocator = renderer->GetVMA();

  VkBufferCreateInfo info {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  info.size = m_Desc.Size;
  info.usage = ToVk(m_Desc.Usage);
  info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocInfo {};
  allocInfo.usage = GetVMAUsage(m_Desc.Usage);
  allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

  m_Allocation = renderer->GetLogicalDevice()->CreateVMABuffer(info, allocInfo);

  if (data) {
    if (m_Desc.Usage == BufferUsage::Staging || m_Desc.Usage == BufferUsage::Uniform) {
      SetData(data);
    } else {
      BufferDescription stageDesc;
      stageDesc.Size = desc.Size;
      stageDesc.Usage = BufferUsage::Staging;
      stageDesc.Name = desc.Name;

      auto stageBuffer = renderer->CreateBuffer(stageDesc);
      stageBuffer->SetData(data);
      VkBufferCopy region[] = {
          {0, 0, m_Desc.Size}
      };
      CopyBufferAttr attr {};
      attr.Src = stageBuffer.get();
      attr.Dst = this;
      renderer->CopyBuffer(attr);
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
  FOO_ASSERT(m_Allocation);
  vmaMapMemory(m_Allocator, m_Allocation, &m_MapPtr);
}

void VulkanBuffer::Unmap() {
  FOO_ASSERT(m_Allocation);
  vmaUnmapMemory(m_Allocator, m_Allocation);
  m_MapPtr = 0;
}

VulkanBuffer::~VulkanBuffer() {
  if (m_MapPtr) {
    Unmap();
  }
}

}  // namespace fg
