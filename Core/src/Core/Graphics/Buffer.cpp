#include "Buffer.h"
#include <vulkan/vulkan_core.h>
#include <cassert>
#include "pch.h"
#include "../Backend/VulkanCheckResult.h"
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
    Buffer::~Buffer()
    {
    }
    void Buffer::Release(VkDevice device)
    {
        (vkDestroyBuffer(device, m_Buffer, nullptr));
        (vkFreeMemory(device, m_Memory, nullptr));
    }
    void Buffer::Create()
    {
        VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        createInfo.size               = m_Size;
        createInfo.usage              = m_Usage;
        createInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

        VK_CALL(vkCreateBuffer(m_Device, &createInfo, 0, &m_Buffer));
        void* data;
        Allocate();
    }
    void Buffer::Allocate()
    {
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(m_Device, m_Buffer, &memoryRequirements);
        uint32_t memoryTypeIndex =
            SelectMemoryType(m_MemoryProperties,
                             memoryRequirements.memoryTypeBits, m_MemoryFlags);
        assert(memoryTypeIndex != ~0u);
        VkMemoryAllocateInfo allocateInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocateInfo.allocationSize        = memoryRequirements.size;
        allocateInfo.memoryTypeIndex       = memoryTypeIndex;
        VkMemoryAllocateFlagsInfo flagInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};

        if (m_Usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        {
            allocateInfo.pNext  = &flagInfo;
            flagInfo.flags      = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
            flagInfo.deviceMask = 1;
        }

        VK_CALL(vkAllocateMemory(m_Device, &allocateInfo, 0, &m_Memory));
        if (m_MemoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            VK_CALL(vkMapMemory(m_Device, m_Memory, 0, m_Size, 0, &m_Data));
        }
    }
    void Buffer::Bind()
    {
        VK_CALL(vkBindBufferMemory(m_Device, m_Buffer, m_Memory, 0));
    }
    void Buffer::SetData(size_t size, void* data)
    {
        if (m_MemoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            std::memcpy(m_Data, data, size);
        }
        else
        {
            vkMapMemory(m_Device, m_Memory, 0, size, 0, &m_Data);
            std::memcpy(m_Data, data, size);
            vkUnmapMemory(m_Device, m_Memory);
        }
    }
    Buffer::Buffer(BufferCreateInfo info)
        : m_Size(info.size),
          m_MemoryProperties(info.memoryProperties),
          m_MemoryFlags(info.memoryFlags),
          m_Device(info.device)
    {
        VkBufferUsageFlags usage{};
        switch (info.usage)
        {
            case FooGame::BufferUsage::VERTEX:
            {
                usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            }
            break;
            case FooGame::BufferUsage::UNIFORM:
            {
                usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            }
            break;
            case FooGame::BufferUsage::INDEX:
            {
                usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }
            break;
        }
        m_Usage = usage;
        Create();
    }
    BufferBuilder::BufferBuilder(VkDevice device)
        : m_Device(device), createInfo({})
    {
        createInfo.device = device;
    }

    BufferBuilder& BufferBuilder::SetInitialSize(size_t size)
    {
        createInfo.size = size;
        return *this;
    }
    BufferBuilder& BufferBuilder::SetMemoryProperties(
        VkPhysicalDeviceMemoryProperties memoryProperties)
    {
        createInfo.memoryProperties = memoryProperties;

        return *this;
    }
    BufferBuilder& BufferBuilder::SetMemoryFlags(VkMemoryPropertyFlags memFlags)
    {
        createInfo.memoryFlags = memFlags;
        return *this;
    }
    BufferBuilder& BufferBuilder::SetUsage(BufferUsage usage)
    {
        createInfo.usage = usage;
        return *this;
    }
    Buffer BufferBuilder::Build()
    {
        Buffer buffer{createInfo};
        return buffer;
    }
}  // namespace FooGame
