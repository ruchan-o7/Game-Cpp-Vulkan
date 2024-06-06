#include "VulkanBuffer.h"
#include <cassert>
#include "TypeConversion.h"
#include "RenderDevice.h"
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
    VulkanBuffer::VulkanBuffer(const BuffDesc& info, const BuffData& data)
        : m_pRenderDevice(info.pRenderDevice),
          m_Buffer(nullptr),
          m_Memory(nullptr),
          m_Data(nullptr),
          m_Size(data.Size),
          m_Usage(info.Usage)
    {
        assert(data.Size > 0);

        Create(data);
    }
    void VulkanBuffer::Release()
    {
        if (m_Buffer != nullptr)
        {
            auto lDev = m_pRenderDevice->GetLogicalDevice();
            lDev->ReleaseVulkanObject(std::move(m_Buffer));
            lDev->ReleaseVulkanObject(std::move(m_Memory));
        }
    }
    void VulkanBuffer::Create(const BuffData& data)
    {
        auto lDev = m_pRenderDevice->GetLogicalDevice();
        auto pDev = m_pRenderDevice->GetPhysicalDevice();
        VkBufferCreateInfo ci{};
        ci.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        ci.pQueueFamilyIndices   = nullptr;
        ci.queueFamilyIndexCount = 0;
        ci.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        ci.usage                 = BuToVkBufferUsage(m_Usage);
        ci.pNext                 = nullptr;
        ci.size                  = m_Size;

        auto TODO = lDev->CreateBuffer(ci, "buffer");

        auto memProps = pDev->GetMemoryProperties();
        auto memReqs  = lDev->GetBufferMemoryRequirements(m_Buffer);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReqs.size;

        uint32_t memIndex = SelectMemoryType(memProps, memReqs.memoryTypeBits, m_MemoryFlags);
        allocInfo.memoryTypeIndex = memIndex;

        VkMemoryAllocateFlagsInfo flagInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
        auto bufferUsage                   = BuToVkBufferUsage(m_Usage);

        if (bufferUsage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        {
            flagInfo.flags      = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
            flagInfo.deviceMask = 1;
            allocInfo.pNext     = &flagInfo;
        }
        auto TODO2 = lDev->AllocateDeviceMemory(allocInfo);
        lDev->BindBufferMemory(m_Buffer, m_Memory, 0);
        lDev->MapMemory(m_Memory, 0, m_Size, m_MemoryFlags, &m_Data);

        memcpy(m_Data, data.Data, m_Size);

        if (!(m_MemoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            lDev->UnmapMemory(m_Memory);
        }
    }

}  // namespace ENGINE_NAMESPACE
