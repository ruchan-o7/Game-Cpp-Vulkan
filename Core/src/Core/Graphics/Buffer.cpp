#include "Buffer.h"
#include <vulkan/vulkan_core.h>
#include <cstring>
#include "../Backend/VulkanCheckResult.h"
#include "../Core/Renderer2D.h"
#include "glm/fwd.hpp"
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

    void Buffer::Release()
    {
        auto device = Renderer2D::GetDevice();
        (vkDestroyBuffer(device, m_Buffer, nullptr));
        (vkFreeMemory(device, m_Memory, nullptr));
    }
    void Buffer::Init(size_t s,
                      const VkPhysicalDeviceMemoryProperties& memoryProperties,
                      VkMemoryPropertyFlags memoryFlags,
                      VkBufferUsageFlags usage)
    {
        m_Size             = s;
        m_MemoryProperties = memoryProperties;
        m_MemoryFlags      = memoryFlags;
        m_Usage            = usage;
        Init();
    }

    void Buffer::Init()
    {
        auto device                   = Renderer2D::GetDevice();
        VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        createInfo.size               = m_Size;
        createInfo.usage              = m_Usage;

        VK_CALL(vkCreateBuffer(device, &createInfo, 0, &m_Buffer));

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, m_Buffer, &memoryRequirements);

        uint32_t memoryTypeIndex =
            SelectMemoryType(m_MemoryProperties,
                             memoryRequirements.memoryTypeBits, m_MemoryFlags);
        assert(memoryTypeIndex != ~0u);

        VkMemoryAllocateInfo allocateInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocateInfo.allocationSize  = memoryRequirements.size;
        allocateInfo.memoryTypeIndex = memoryTypeIndex;

        VkMemoryAllocateFlagsInfo flagInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};

        if (m_Usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        {
            allocateInfo.pNext  = &flagInfo;
            flagInfo.flags      = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
            flagInfo.deviceMask = 1;
        }

        VK_CALL(vkAllocateMemory(device, &allocateInfo, 0, &m_Memory));

        VK_CALL(vkBindBufferMemory(device, m_Buffer, m_Memory, 0));

        if (m_MemoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            VK_CALL(vkMapMemory(device, m_Memory, 0, m_Size, 0, &m_Data));
        }
    }
    void Buffer::SetData(void* data, size_t size)
    {
        auto device = Renderer2D::GetDevice();

        if (m_MemoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            std::memcpy(m_Data, data, size);
        }
        else
        {
            vkMapMemory(device, m_Memory, 0, size, 0, &m_Data);
            std::memcpy(m_Data, data, size);
            vkUnmapMemory(device, m_Memory);
        }
    }

    Buffer::Buffer(size_t s,
                   const VkPhysicalDeviceMemoryProperties& memoryProperties,
                   VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags)
        : m_Size(s),
          m_MemoryProperties(memoryProperties),
          m_MemoryFlags(memoryFlags),
          m_Usage(usage)
    {
    }
}  // namespace FooGame
