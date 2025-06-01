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
    VulkanBuffer(const BufferDescription& desc, std::weak_ptr<Renderer> renderer,
                 const Buffer data = Buffer());
    virtual ~VulkanBuffer();

    const BufferDescription& GetDesc() const {
      return m_Desc;
    }
    void Map(VkMemoryMapFlags flags = 0);
    void Unmap();
    void CopyTo(VulkanBuffer* destination, VkBufferCopy* regions, uint32_t regionCount);
    void SetData(Buffer buffer);

    VkBuffer GetVkBuffer() const {
      return m_Handle;
    }

  private:
    BufferDescription m_Desc;
    ResourceState m_State = ResourceState::Unknown;
    std::weak_ptr<Renderer> m_Renderer;
    BufferWrapper m_Handle;
    MemoryWrapper m_Memory;
    void* m_MapPtr = nullptr;
};

}  // namespace fg
