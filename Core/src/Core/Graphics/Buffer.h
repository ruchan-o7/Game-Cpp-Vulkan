#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../Core/Base.h"
#include "Core/Graphics/Device.h"
namespace FooGame
{
    enum class BufferUsage
    {
        VERTEX,
        INDEX,
        UNIFORM,
    };
    struct BufferCreateInfo
    {
            VkDevice device;
            size_t size;
            VkPhysicalDeviceMemoryProperties memoryProperties;
            VkMemoryPropertyFlags memoryFlags;
            BufferUsage usage;
    };
    class Buffer
    {
        public:
            Buffer(BufferCreateInfo info);
            ~Buffer();
            void Release(VkDevice device);
            void Allocate();
            void Bind();
            void SetData(size_t size, void* data);
            VkBuffer* GetBuffer() { return &m_Buffer; }

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
            VkDevice m_Device;
    };
    class BufferBuilder
    {
        public:
            BufferBuilder(VkDevice device);
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
