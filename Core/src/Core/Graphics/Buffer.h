#pragma once
#include <vulkan/vulkan.h>
#include "../Core/Base.h"
namespace FooGame
{
    class Buffer
    {
        public:
            Buffer()  = default;
            ~Buffer() = default;
            Buffer(size_t s,
                   const VkPhysicalDeviceMemoryProperties& memoryProperties,
                   VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags);
            void Release();
            void Init();
            void Init(size_t s,
                      const VkPhysicalDeviceMemoryProperties& memoryProperties,
                      VkMemoryPropertyFlags memoryFlags,
                      VkBufferUsageFlags usage);
            static Shared<Buffer> Create(
                size_t s,
                const VkPhysicalDeviceMemoryProperties& memoryProperties,
                VkMemoryPropertyFlags memoryFlags, VkBufferUsageFlags usage);
            VkBuffer* GetBuffer() { return &m_Buffer; }
            void SetData(void* data, size_t size);

        private:
            VkBuffer m_Buffer;
            VkDeviceMemory m_Memory;
            void* m_Data;
            size_t m_Size;
            VkPhysicalDeviceMemoryProperties m_MemoryProperties;
            VkMemoryPropertyFlags m_MemoryFlags;
            VkBufferUsageFlags m_Usage;
    };
}  // namespace FooGame
