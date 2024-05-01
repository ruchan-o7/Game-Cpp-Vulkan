#pragma once
#include <vulkan/vulkan.h>

namespace FooGame
{
    class Buffer
    {
        public:
            Buffer(size_t size,
                   const VkPhysicalDeviceMemoryProperties& memProps,
                   VkBufferUsageFlags usage, VkMemoryPropertyFlags flags,
                   void* data = 0);
            ~Buffer();
            [[nodiscard]] void* GetData();
            [[nodiscard]] VkBuffer GetBuffer();
            void SetData(void* data, size_t size);

        private:
            VkBuffer m_Buffer;
            VkDeviceMemory m_Memory;
            void* m_Data;
            size_t m_Size;
            bool isMapped = false;
    };
}  // namespace FooGame
