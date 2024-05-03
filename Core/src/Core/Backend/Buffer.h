#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace FooGame
{
    struct Buffer
    {
            VkBuffer buf;
            VkDeviceMemory mem;
            void* data;
            size_t size;
    };
    void CreateBuffer(Buffer& result, VkDevice device,
                      const VkPhysicalDeviceMemoryProperties& memoryProperties,
                      size_t size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags memoryFlags);

    void CreateVertexBuffer(
        Buffer& result, VkDevice device,
        const VkPhysicalDeviceMemoryProperties& memoryProperties, size_t size,
        VkMemoryPropertyFlags memoryFlags);
    void CreateIndexBuffer(
        Buffer& result, VkDevice device,
        const VkPhysicalDeviceMemoryProperties& memoryProperties, size_t size,
        VkMemoryPropertyFlags memoryFlags);

    void UploadBuffer(VkDevice device, VkCommandPool commandPool,
                      VkCommandBuffer commandBuffer, VkQueue queue,
                      const Buffer& buffer, const Buffer& scratch,
                      const void* data, size_t size);
    void UpdateBufferData(Buffer& buffer, void* data, size_t size);
}  // namespace FooGame
