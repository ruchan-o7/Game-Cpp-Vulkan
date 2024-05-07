#pragma once
#include <vulkan/vulkan.h>
#include "../Core/Base.h"
#include "Core/Graphics/Device.h"
#include "Core/Graphics/Image.h"
#include "vulkan/vulkan_core.h"
namespace FooGame
{
    enum class BufferUsage
    {
        VERTEX,
        INDEX,
        UNIFORM,
        TRANSFER_SRC,
        TRANSFER_DST,
    };
    struct BufferCreateInfo
    {
            size_t size;
            VkPhysicalDeviceMemoryProperties memoryProperties;
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
            VkPhysicalDeviceMemoryProperties m_MemoryProperties;
            VkMemoryPropertyFlags m_MemoryFlags;
            VkBufferUsageFlags m_Usage;
    };
    class BufferBuilder
    {
        public:
            BufferBuilder()  = default;
            ~BufferBuilder() = default;
            BufferBuilder& SetInitialSize(size_t size);
            BufferBuilder& SetMemoryProperties(
                VkPhysicalDeviceMemoryProperties memoryProperties);
            BufferBuilder& SetMemoryFlags(VkMemoryPropertyFlags memFlags);
            BufferBuilder& SetUsage(BufferUsage usage);
            Buffer Build();

        private:
            VkDevice m_Device;
            BufferCreateInfo createInfo;
    };
}  // namespace FooGame
