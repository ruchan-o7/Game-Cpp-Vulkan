#include "Buffer.h"
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include "../Backend/VulkanCheckResult.h"
#include "../Core/Base.h"
#include "src/Core/Core/Renderer2D.h"
namespace FooGame
{
    static u32 SelectMemoryType(
        const VkPhysicalDeviceMemoryProperties& memProps, u32 memoryTypeBits,
        VkMemoryPropertyFlags flags)
    {
        for (u32 i = 0; i < memProps.memoryTypeCount; ++i)
        {
            if ((memoryTypeBits & (1 << i)) != 0 &&
                (memProps.memoryTypes[i].propertyFlags & flags) == flags)
            {
                return i;
            }
        }
        return ~0u;
    }
    Buffer::Buffer(size_t size,
                   const VkPhysicalDeviceMemoryProperties& memProps,
                   VkBufferUsageFlags usage, VkMemoryPropertyFlags flags,
                   void* data)
        : m_Buffer(0), m_Memory(0), m_Data(data), m_Size(size)
    {
        auto device = Renderer2D::GetDevice();
        VkBufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.size  = size;
        createInfo.usage = usage;
        VK_CALL(vkCreateBuffer(device, &createInfo, nullptr, &m_Buffer));

        VkMemoryRequirements memReqs{};
        vkGetBufferMemoryRequirements(device, m_Buffer, &memReqs);
        u32 memoryTypeIndex =
            SelectMemoryType(memProps, memReqs.memoryTypeBits, flags);
        assert(memoryTypeIndex != ~0u);
        VkMemoryAllocateInfo allocateInfo{};
        allocateInfo.allocationSize  = memReqs.size;
        allocateInfo.memoryTypeIndex = memoryTypeIndex;
        allocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

        VkMemoryAllocateFlagsInfo flagInfo{};
        flagInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        {
            allocateInfo.pNext  = &flagInfo;
            flagInfo.flags      = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
            flagInfo.deviceMask = 1;
        }
        VK_CALL(vkAllocateMemory(device, &allocateInfo, nullptr, &m_Memory));
        VK_CALL(vkBindBufferMemory(device, m_Buffer, m_Memory, 0));
        if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            VK_CALL(vkMapMemory(device, m_Memory, 0, m_Size, 0, &data));
            isMapped = true;
        }
    }
    void Buffer::SetData(void* data, size_t size)
    {
        auto device = Renderer2D::GetDevice();
        if (m_Size < size)
        {
            std::cerr << "Size of data is bigger than allocated size\n";
            return;
        }
        // if (!isMapped)
        // {
        VK_CALL(vkMapMemory(device, m_Memory, 0, m_Size, 0, &m_Data));
        isMapped = true;
        // }

        // std::memcpy(data, m_Data, size);
        std::memcpy(m_Data, data, size);
        vkUnmapMemory(device, m_Memory);
        isMapped = false;
    }
    VkBuffer Buffer::GetBuffer()
    {
        return m_Buffer;
    }
    Buffer::~Buffer()
    {
        auto device = Renderer2D::GetDevice();
        vkDestroyBuffer(device, m_Buffer, nullptr);
        vkFreeMemory(device, m_Memory, nullptr);
    }
}  // namespace FooGame
