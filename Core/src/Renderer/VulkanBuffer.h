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
    virtual ~VulkanBuffer() = default;

    const BufferDescription& GetDesc() const {
      return m_Desc;
    }
    void Map(VkMemoryMapFlags flags = 0);
    void Unmap();
    void CopyData(Buffer buffer);

    VkBuffer GetVkBuffer() const {
      return m_Handle;
    }

  private:
    BufferDescription m_Desc;
    ResourceState m_State = ResourceState::Unknown;
    std::weak_ptr<Renderer> m_Renderer;
    BufferWrapper m_Handle;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;
    void* m_MapPtr = nullptr;
};

}  // namespace fg
