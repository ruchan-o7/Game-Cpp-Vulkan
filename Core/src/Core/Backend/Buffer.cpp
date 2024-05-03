#include "Buffer.h"
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include "../Backend/VulkanCheckResult.h"
#include "../Core/Base.h"
namespace FooGame
{
    static u32 SelectMemoryType(
        const VkPhysicalDeviceMemoryProperties& memoryProperties,
        u32 memoryTypeBits, VkMemoryPropertyFlags flags)
    {
        for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i)
        {
            if ((memoryTypeBits & (1 << i)) != 0 &&
                (memoryProperties.memoryTypes[i].propertyFlags & flags) ==
                    flags)
            {
                return i;
            }
        }

        assert(!"No compatible memory type found");
        return ~0u;
    }
    void CreateBuffer(Buffer& result, VkDevice device,
                      const VkPhysicalDeviceMemoryProperties& memoryProperties,
                      size_t size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags memoryFlags)
    {
        VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        createInfo.size               = size;
        createInfo.usage              = usage;

        VkBuffer buffer = 0;
        VK_CALL(vkCreateBuffer(device, &createInfo, 0, &buffer));

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

        uint32_t memoryTypeIndex = SelectMemoryType(
            memoryProperties, memoryRequirements.memoryTypeBits, memoryFlags);
        assert(memoryTypeIndex != ~0u);

        VkMemoryAllocateInfo allocateInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocateInfo.allocationSize  = memoryRequirements.size;
        allocateInfo.memoryTypeIndex = memoryTypeIndex;

        VkMemoryAllocateFlagsInfo flagInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};

        if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        {
            allocateInfo.pNext  = &flagInfo;
            flagInfo.flags      = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
            flagInfo.deviceMask = 1;
        }

        VkDeviceMemory memory = 0;
        VK_CALL(vkAllocateMemory(device, &allocateInfo, 0, &memory));

        VK_CALL(vkBindBufferMemory(device, buffer, memory, 0));

        void* data = 0;
        if (memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            VK_CALL(vkMapMemory(device, memory, 0, size, 0, &data));
        }

        result.buf  = buffer;
        result.mem  = memory;
        result.data = data;
        result.size = size;
    }

    void CreateVertexBuffer(
        Buffer& result, VkDevice device,
        const VkPhysicalDeviceMemoryProperties& memoryProperties, size_t size,
        VkMemoryPropertyFlags memoryFlags)
    {
        CreateBuffer(result, device, memoryProperties, size,
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, memoryFlags);
    }
    void CreateIndexBuffer(
        Buffer& result, VkDevice device,
        const VkPhysicalDeviceMemoryProperties& memoryProperties, size_t size,
        VkMemoryPropertyFlags memoryFlags)
    {
        CreateBuffer(result, device, memoryProperties, size,
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT, memoryFlags);
    }

    void UploadBuffer(VkDevice device, VkCommandPool commandPool,
                      VkCommandBuffer commandBuffer, VkQueue queue,
                      const Buffer& buffer, const Buffer& scratch,
                      const void* data, size_t size)
    {
        assert(size > 0);
        assert(scratch.data);
        assert(scratch.size >= size);
        memcpy(scratch.data, data, size);

        VK_CALL(vkResetCommandPool(device, commandPool, 0));

        VkCommandBufferBeginInfo beginInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        VkBufferCopy region = {0, 0, VkDeviceSize(size)};
        vkCmdCopyBuffer(commandBuffer, scratch.buf, buffer.buf, 1, &region);

        VK_CALL(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo       = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        VK_CALL(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

        VK_CALL(vkDeviceWaitIdle(device));
    }
    void UpdateBufferData(Buffer& buffer, void* data, size_t size)
    {
        memcpy(buffer.data, data, size);
    }
}  // namespace FooGame
