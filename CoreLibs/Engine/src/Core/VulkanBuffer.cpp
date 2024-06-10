#include "VulkanBuffer.h"
#include <cassert>
#include "TypeConversion.h"
#include "RenderDevice.h"
#include "Types.h"
#include "vulkan/vulkan_core.h"
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
        if (m_Desc.Usage == Vulkan::BUFFER_USAGE_UNIFORM)
        {
            if (m_Desc.MemoryFlag == Vulkan::BUFFER_MEMORY_FLAG_CPU_VISIBLE)
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

        VkBufferUsageFlags usage           = m_Desc.Usage;  // BuffUsageToVkUsage(m_Desc.Usage);

        VkBufferCreateInfo ci{};
        ci.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        ci.size                  = m_BufferData.Size;
        ci.usage                 = m_Desc.Usage;
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
            memReqs.memoryTypeBits, m_Desc.MemoryFlag);

        m_Memory = std::move(lDev->AllocateDeviceMemory(allocInfo));

        lDev->BindBufferMemory(m_Buffer, m_Memory, 0);

     
    }
    void VulkanBuffer::MapMemory()
    {
        m_Desc.pRenderDevice->GetLogicalDevice()->MapMemory(m_Memory, 0, m_BufferData.Size,
                                                            0, &m_MappedPtr);
    }
    void VulkanBuffer::UnMapMemory()
    {
        if (m_Desc.MemoryFlag == Vulkan::BUFFER_MEMORY_FLAG_GPU_ONLY)
        {
            m_Desc.pRenderDevice->GetLogicalDevice()->UnmapMemory(m_Memory);
        }
    }
    void VulkanBuffer::UpdateData(void* data, size_t size, size_t offset)
    {
        assert(size <= m_BufferData.Size);
        if (m_Desc.MemoryFlag == Vulkan::BUFFER_MEMORY_FLAG_CPU_VISIBLE)
        {
            memcpy(m_MappedPtr, m_BufferData.Data, m_BufferData.Size);
        }
        else
        {
            m_Desc.pRenderDevice->GetLogicalDevice()->MapMemory(m_Memory, offset, size,
                                                                (m_Desc.MemoryFlag), &data);
            memcpy(m_MappedPtr, data, size);
            m_Desc.pRenderDevice->GetLogicalDevice()->UnmapMemory(m_Memory);
            // vkUnmapMemory(device, m_Memory);
        }
    }

    std::unique_ptr<VulkanBuffer> CreateDynamicBuffer(size_t size, Vulkan::BUFFER_USAGE usage)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }
    std::unique_ptr<VulkanBuffer> CreateVertexBuffer(VulkanBuffer::BuffData& data,
                                                     RenderDevice* pRenderDevice,
                                                     const char* name = "Vertex buffer")
    {
        // size_t size = data.Size;
        // VulkanBuffer::BuffDesc stageDesc{};
        // stageDesc.name          = "Stage buffer";
        // stageDesc.Usage         = Vulkan::BUFFER_USAGE::TransferSource;
        // stageDesc.MemoryFlag    = Vulkan::BUFFER_MEMORY_FLAG::CpuVisible;
        // stageDesc.pRenderDevice = pRenderDevice;
        // VulkanBuffer stageBuffer{stageDesc, std::move(data)};
        //
        // VulkanBuffer::BuffDesc vertexBufferDesc{};
        // vertexBufferDesc.name          = name;
        // vertexBufferDesc.Usage         = Vulkan::BUFFER_USAGE::TransferSource;
        // vertexBufferDesc.MemoryFlag    = Vulkan::BUFFER_MEMORY_FLAG::CpuVisible;
        // vertexBufferDesc.pRenderDevice = pRenderDevice;
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
