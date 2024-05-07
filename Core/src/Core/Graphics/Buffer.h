#pragma once
#include <vulkan/vulkan.h>
#include "../Backend/Vertex.h"
namespace FooGame
{
    struct Image;
    enum class BufferUsage
    {
        VERTEX,
        INDEX,
        UNIFORM,
        TRANSFER_SRC,
        TRANSFER_DST,
        TRANSFER_DST_VERTEX,
        TRANSFER_DST_INDEX,
    };
    struct BufferCreateInfo
    {
            size_t size;
            VkMemoryPropertyFlags memoryFlags;
            BufferUsage usage;
    };
    class Buffer
    {
        public:
            Buffer(BufferCreateInfo info);
            Buffer(const Buffer& other) = delete;
            Buffer(Buffer&& other);
            ~Buffer() = default;
            void Release();
            void Allocate();
            void Bind();
            void SetData(size_t size, void* data);
            VkBuffer* GetBuffer() { return &m_Buffer; }
            void CopyTo(Buffer& target, VkDeviceSize size);
            void CopyToImage(Image& image);

        private:
            void Create();

        private:
            VkBuffer m_Buffer;
            VkDeviceMemory m_Memory;
            void* m_Data;
            size_t m_Size;
            VkMemoryPropertyFlags m_MemoryFlags;
            VkBufferUsageFlags m_Usage;
    };
    class BufferBuilder
    {
        public:
            BufferBuilder()  = default;
            ~BufferBuilder() = default;
            BufferBuilder& SetInitialSize(size_t size);
            BufferBuilder& SetMemoryFlags(VkMemoryPropertyFlags memFlags);
            BufferBuilder& SetUsage(BufferUsage usage);
            Buffer Build();

        private:
            VkDevice m_Device;
            BufferCreateInfo createInfo;
    };
    Buffer CreateVertexBuffer(const List<Vertex> vertices);
    Buffer CreateIndexBuffer(const List<u32> indices);
    static VkBufferUsageFlags ParseBufferUsage(BufferUsage usage);
}  // namespace FooGame
