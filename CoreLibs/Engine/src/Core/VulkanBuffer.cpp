#include "VulkanBuffer.h"
#include <cassert>
#include "TypeConversion.h"
#include "RenderDevice.h"
#include "Types.h"
namespace ENGINE_NAMESPACE
{

    static uint32_t SelectMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties,
                                     uint32_t memoryTypeBits, VkMemoryPropertyFlags flags)
    {
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
        {
            if ((memoryTypeBits & (1 << i)) != 0 &&
                (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
            {
                return i;
            }
        }

        assert(!"No compatible memory type found");
        return ~0u;
    }
    VulkanBuffer::VulkanBuffer(const BuffDesc& info, BuffData&& data)
        : m_Desc(info), m_BufferData(std::move(data)), m_Buffer(nullptr), m_Memory(nullptr)
    {
        if (m_Desc.Usage == Vulkan::BufferUsage::Uniform)
        {
            if (m_Desc.MemoryFlag == Vulkan::BufferMemoryFlag::CpuVisible)
            {
                assert("Uniform buffer should cpu visible");
            }
        }
        assert(m_BufferData.Size > 0);
        Init();
    }
    void VulkanBuffer::Release()
    {
        if (m_Buffer != nullptr)
        {
            // auto lDev = m_Desc.pRenderDevice->GetLogicalDevice();
            m_Buffer.Release();
            m_Memory.Release();
            // lDev->ReleaseVulkanObject(std::move(m_Buffer));
            // lDev->ReleaseVulkanObject(std::move(m_Memory));
        }
    }
    void VulkanBuffer::Init()
    {
        auto lDev = m_Desc.pRenderDevice->GetLogicalDevice();
        auto pDev = m_Desc.pRenderDevice->GetPhysicalDevice();

        VkBufferUsageFlags usage           = BuffUsageToVkUsage(m_Desc.Usage);
        VkMemoryPropertyFlags memPropFlags = BufMemFlagToVkFlag(m_Desc.MemoryFlag);

        VkBufferCreateInfo ci{};
        ci.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        ci.size                  = m_BufferData.Size;
        ci.usage                 = usage;
        ci.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        ci.pNext                 = nullptr;
        ci.pQueueFamilyIndices   = nullptr;
        ci.queueFamilyIndexCount = 0;

        m_Buffer = std::move(lDev->CreateBuffer(ci, m_Desc.name));

        auto memReqs  = lDev->GetBufferMemoryRequirements(m_Buffer);
        auto memProps = pDev->GetMemoryProperties();

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memReqs.size;
        allocInfo.memoryTypeIndex = m_Desc.pRenderDevice->GetPhysicalDevice()->FindMemoryType(
            memReqs.memoryTypeBits, memPropFlags);

        m_Memory = std::move(lDev->AllocateDeviceMemory(allocInfo));

        lDev->BindBufferMemory(m_Buffer, m_Memory, 0);

        MapMemory();
        if (m_Desc.Usage != Vulkan::BufferUsage::Uniform)
        {
            memcpy(m_MappedPtr, m_BufferData.Data, m_BufferData.Size);

            UnMapMemory();
        }
    }
    void VulkanBuffer::MapMemory()
    {
        VkMemoryPropertyFlags memPropFlags = BufMemFlagToVkFlag(m_Desc.MemoryFlag);
        m_Desc.pRenderDevice->GetLogicalDevice()->MapMemory(m_Memory, 0, m_BufferData.Size,
                                                            memPropFlags, &m_MappedPtr);
    }
    void VulkanBuffer::UnMapMemory()
    {
        if (m_Desc.MemoryFlag == Vulkan::BufferMemoryFlag::GpuOnly)
        {
            m_Desc.pRenderDevice->GetLogicalDevice()->UnmapMemory(m_Memory);
        }
    }
    void VulkanBuffer::UpdateData(void* data, size_t size, size_t offset)
    {
        assert(size <= m_BufferData.Size);
        if (m_Desc.MemoryFlag == Vulkan::BufferMemoryFlag::CpuVisible)
        {
            memcpy(m_MappedPtr, m_BufferData.Data, m_BufferData.Size);
        }
        else
        {
            m_Desc.pRenderDevice->GetLogicalDevice()->MapMemory(
                m_Memory, offset, size, BufMemFlagToVkFlag(m_Desc.MemoryFlag), &data);
            memcpy(m_MappedPtr, data, size);
            m_Desc.pRenderDevice->GetLogicalDevice()->UnmapMemory(m_Memory);
            // vkUnmapMemory(device, m_Memory);
        }
    }

    std::unique_ptr<VulkanBuffer> CreateDynamicBuffer(size_t size, Vulkan::BufferUsage usage)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }
    std::unique_ptr<VulkanBuffer> CreateVertexBuffer(const BuffData& data)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }
    std::unique_ptr<VulkanBuffer> CreateIndexBuffer(const BuffData& data)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    void VulkanBuffer::CopyTo(VulkanBuffer& destination, size_t size, VkCommandPool commandPool,
                              VkQueue queue)
    {
        auto device = m_Desc.pRenderDevice->GetLogicalDevice()->GetVkDevice();
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool        = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, GetBuffer(), destination.GetBuffer(), 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }
}  // namespace ENGINE_NAMESPACE
