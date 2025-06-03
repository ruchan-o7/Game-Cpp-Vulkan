#pragma once
#include "../Core/Ref.h"
#include "../Core/Buffer.h"
#include "../Renderer/Misc.h"
#include "../Renderer/VulkanObject.h"

namespace fg {

class Renderer;

enum class BufferUsage {
  None = 0,
  Vertex,
  Index,
  Uniform,
  Staging,
};

struct BufferDescription {
    const char* Name = nullptr;
    uint64_t Size = 0;
    BufferUsage Usage = BufferUsage::None;
};

class VulkanBuffer : public RefBase {
  public:
    VulkanBuffer(const BufferDescription& desc, Renderer* renderer, const Buffer data = Buffer());
    virtual ~VulkanBuffer();

    const BufferDescription& GetDesc() const {
      return m_Desc;
    }
    void Map(VkMemoryMapFlags flags = 0);
    void Unmap();
    void SetData(Buffer buffer);

    VkBuffer GetVkBuffer() const {
      return m_Allocation;
    }

  private:
    BufferDescription m_Desc;
    ResourceState m_State = ResourceState::Unknown;
    VmaBufferWrapper m_Allocation;
    VmaAllocator m_Allocator;
    void* m_MapPtr = nullptr;
};

}  // namespace fg
