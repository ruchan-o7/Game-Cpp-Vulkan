#include "Buffer.h"
#include <vulkan/vulkan_core.h>
#include "../Core/Engine.h"
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

    void Buffer::Release()
    {
        auto device = Engine::Get()->GetDevice().GetDevice();
        (vkDestroyBuffer(device, m_Buffer, nullptr));
        (vkFreeMemory(device, m_Memory, nullptr));
    }
    void Buffer::Create()
    {
        auto device                   = Engine::Get()->GetDevice().GetDevice();
        VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        createInfo.size               = m_Size;
        createInfo.usage              = m_Usage;
        createInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

        VK_CALL(vkCreateBuffer(device, &createInfo, 0, &m_Buffer));
        void* data;
        // Allocate();
    }
    void Buffer::Allocate()
    {
        assert(m_Size != 0 && "Buffer size must be larger than 0");
        auto device             = Engine::Get()->GetDevice();
        auto dev                = device.GetDevice();
        auto memoryRequirements = device.GetMemoryRequirements(m_Buffer);
        u32 memoryTypeIndex =
            SelectMemoryType(Engine::Get()->GetDevice().GetMemoryProperties(),
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

        VK_CALL(
            vkAllocateMemory(device.GetDevice(), &allocateInfo, 0, &m_Memory));
        if (m_MemoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            VK_CALL(vkMapMemory(device.GetDevice(), m_Memory, 0, m_Size, 0,
                                &m_Data));
        }
    }
    void Buffer::Bind()
    {
        auto device = Engine::Get()->GetDevice().GetDevice();
        VK_CALL(vkBindBufferMemory(device, m_Buffer, m_Memory, 0));
    }
    void Buffer::SetData(size_t size, void* data)
    {
        auto device = Engine::Get()->GetDevice().GetDevice();
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
    Buffer::Buffer(BufferCreateInfo info)
        : m_Size(info.size), m_MemoryFlags(info.memoryFlags)
    {
        m_Usage = ParseBufferUsage(info.usage);
        Create();
    }
    void Buffer::CopyTo(Buffer& target, VkDeviceSize size)
    {
        auto cmd = Engine::Get()->BeginSingleTimeCommands();
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(cmd, m_Buffer, *target.GetBuffer(), 1, &copyRegion);
        Engine::Get()->EndSingleTimeCommands(cmd);
    }
    Buffer::Buffer(Buffer&& other)
    {
        this->m_Data        = other.m_Data;
        this->m_Memory      = other.m_Memory;
        this->m_Size        = other.m_Size;
        this->m_MemoryFlags = other.m_MemoryFlags;
        this->m_Usage       = other.m_Usage;
        this->m_Buffer      = other.m_Buffer;
    }

    void Buffer::CopyToImage(Image& image)

    {
        auto cmd = Engine::Get()->BeginSingleTimeCommands();
        VkBufferImageCopy region{};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel   = 0;
        region.imageSubresource.layerCount = 1;
        region.imageExtent                 = {image.width, image.height, 1};
        vkCmdCopyBufferToImage(cmd, m_Buffer, image.Image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &region);
        Engine::Get()->EndSingleTimeCommands(cmd);
    }

    BufferBuilder& BufferBuilder::SetInitialSize(size_t size)
    {
        createInfo.size = size;
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
        return std::move(buffer);
    }

    Buffer CreateIndexBuffer(const List<u32> indices)
    {
        size_t bufferSize = sizeof(indices[0]) * indices.size();

        auto stagingBuffer =
            BufferBuilder()
                .SetUsage(BufferUsage::TRANSFER_SRC)
                .SetInitialSize(bufferSize)
                .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                .Build();
        stagingBuffer.Allocate();
        stagingBuffer.Bind();
        stagingBuffer.SetData(bufferSize, (void*)indices.data());

        auto indexBuffer =
            BufferBuilder()
                .SetUsage(BufferUsage::TRANSFER_DST_INDEX)
                .SetInitialSize(bufferSize)
                .SetMemoryFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                .Build();
        indexBuffer.Allocate();
        indexBuffer.Bind();

        stagingBuffer.CopyTo(indexBuffer, bufferSize);
        stagingBuffer.Release();
        return std::move(indexBuffer);
    }

    Buffer CreateVertexBuffer(const List<Vertex> vertices)
    {
        size_t bufferSize = sizeof(vertices[0]) * vertices.size();

        auto stagingBuffer =
            BufferBuilder()
                .SetUsage(BufferUsage::TRANSFER_SRC)
                .SetInitialSize(bufferSize)
                .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                .Build();
        stagingBuffer.Allocate();
        stagingBuffer.Bind();
        stagingBuffer.SetData(bufferSize, (void*)vertices.data());

        auto vertexBuffer =
            BufferBuilder()
                .SetUsage(BufferUsage::TRANSFER_DST_VERTEX)
                .SetInitialSize(bufferSize)
                .SetMemoryFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                .Build();
        vertexBuffer.Allocate();
        vertexBuffer.Bind();
        stagingBuffer.CopyTo(vertexBuffer, bufferSize);
        stagingBuffer.Release();
        return std::move(vertexBuffer);
    }

    VkBufferUsageFlags ParseBufferUsage(BufferUsage usage)
    {
        VkBufferUsageFlags usageFlag{};
        switch (usage)
        {
            case FooGame::BufferUsage::VERTEX:
            {
                usageFlag = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            }
            break;
            case FooGame::BufferUsage::UNIFORM:
            {
                usageFlag = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            }
            break;
            case FooGame::BufferUsage::INDEX:
            {
                usageFlag = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }
            break;
            case FooGame::BufferUsage::TRANSFER_SRC:
            {
                usageFlag = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            }
            break;
            case FooGame::BufferUsage::TRANSFER_DST:
            {
                usageFlag = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            break;

            case FooGame::BufferUsage::TRANSFER_DST_VERTEX:
            {
                usageFlag = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            }
            break;

            case FooGame::BufferUsage::TRANSFER_DST_INDEX:
            {
                usageFlag = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }
            break;
        }
        return usageFlag;
    }
}  // namespace FooGame
