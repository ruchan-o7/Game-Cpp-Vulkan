#include "VulkanPhysicalDevice.h"

#include <cassert>
#include "Utils/IndexWrapper.h"
#include "VulkanLogicalDevice.h"
#include <Log.h>
namespace ENGINE_NAMESPACE
{
    std::unique_ptr<VulkanPhysicalDevice> VulkanPhysicalDevice::Create(
        const VulkanPhysicalDevice::CreateInfo& ci)
    {
        auto device = new VulkanPhysicalDevice(ci);
        return std::unique_ptr<VulkanPhysicalDevice>(device);
    }
    VulkanPhysicalDevice::VulkanPhysicalDevice(const VulkanPhysicalDevice::CreateInfo& ci)
        : m_VkDevice(ci.vkDevice)
    {
        assert(m_VkDevice != nullptr);
        vkGetPhysicalDeviceProperties(m_VkDevice, &m_Properties);
        vkGetPhysicalDeviceFeatures(m_VkDevice, &m_Features);
        vkGetPhysicalDeviceMemoryProperties(m_VkDevice, &m_MemoryProps);
        uint32_t QueueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_VkDevice, &QueueFamilyCount, nullptr);
        assert(QueueFamilyCount > 0);
        m_QueueFamilyProperties.resize(QueueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_VkDevice, &QueueFamilyCount,
                                                 m_QueueFamilyProperties.data());
        assert(QueueFamilyCount == m_QueueFamilyProperties.size());

        // Get list of supported extensions
        uint32_t ExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(m_VkDevice, nullptr, &ExtensionCount, nullptr);
        if (ExtensionCount > 0)
        {
            m_SupportedExtensions.resize(ExtensionCount);
            auto res = vkEnumerateDeviceExtensionProperties(m_VkDevice, nullptr, &ExtensionCount,
                                                            m_SupportedExtensions.data());
            assert(res == VK_SUCCESS);
            (void)res;
            assert(ExtensionCount == m_SupportedExtensions.size());
        }

        if (ci.printExtensions && !m_SupportedExtensions.empty())
        {
            FOO_ENGINE_INFO("Extensions supported by device '{0}' : {1}", m_Properties.deviceName,
                            PrintExtensionsList(m_SupportedExtensions));
        }
    }

    bool VulkanPhysicalDevice::IsExtensionSupported(const char* ExtensionName) const
    {
        for (const auto& Extension : m_SupportedExtensions)
        {
            if (strcmp(Extension.extensionName, ExtensionName) == 0)
            {
                return true;
            }
        }

        return false;
    }
    HardwareQueueIndex VulkanPhysicalDevice::FindQueueFamily(VkQueueFlags queueFlag)
    {
        VkQueueFlags QueueFlagsOpt = queueFlag;
        if (queueFlag & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
        {
            queueFlag     &= ~VK_QUEUE_TRANSFER_BIT;
            QueueFlagsOpt  = queueFlag | VK_QUEUE_TRANSFER_BIT;
        }

        static constexpr uint32_t InvalidFamilyInd = std::numeric_limits<uint32_t>::max();
        uint32_t FamilyInd                         = InvalidFamilyInd;

        for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); ++i)
        {
            // First try to find a queue, for which the flags match exactly
            // (i.e. dedicated compute or transfer queue)
            const auto& Props = m_QueueFamilyProperties[i];
            if (Props.queueFlags == queueFlag || Props.queueFlags == QueueFlagsOpt)
            {
                FamilyInd = i;
                break;
            }
        }

        if (FamilyInd == InvalidFamilyInd)
        {
            for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); ++i)
            {
                // Try to find a queue for which all requested flags are set
                const auto& Props = m_QueueFamilyProperties[i];
                // Check only queueFlag as VK_QUEUE_TRANSFER_BIT is
                // optional for graphics and/or compute queues
                if ((Props.queueFlags & queueFlag) == queueFlag)
                {
                    FamilyInd = i;
                    break;
                }
            }
        }

        if (FamilyInd != InvalidFamilyInd)
        {
            if (queueFlag & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
            {
#ifdef FOO_DEBUG
                const auto& Props = m_QueueFamilyProperties[FamilyInd];
                // Queues supporting graphics and/or compute operations must report (1,1,1)
                // in minImageTransferGranularity, meaning that there are no additional restrictions
                // on the granularity of image transfer operations for these queues (4.1).
                assert(Props.minImageTransferGranularity.width == 1 &&
                       Props.minImageTransferGranularity.height == 1 &&
                       Props.minImageTransferGranularity.depth == 1);
#endif
            }
        }
        else
        {
            FOO_ENGINE_CRITICAL("Failed to find suitable queue family");
        }
        return HardwareQueueIndex{FamilyInd};
    }
    uint32_t VulkanPhysicalDevice::GetMemoryTypeIndex(uint32_t typeBits,
                                                      VkMemoryPropertyFlags props) const
    {
        for (uint32_t memoryIndex = 0; memoryIndex < m_MemoryProps.memoryTypeCount; memoryIndex++)
        {
            const uint32_t memoryTypeBit    = (1 << memoryIndex);
            const bool isRequiredMemoryType = (typeBits & memoryTypeBit) != 0;
            if (isRequiredMemoryType)
            {
                const VkMemoryPropertyFlags properties =
                    m_MemoryProps.memoryTypes[memoryIndex].propertyFlags;
                const bool hasRequiredProperties = (properties & props) == props;

                if (hasRequiredProperties)
                {
                    return memoryIndex;
                }
            }
        }
        return InvalidMemoryTypeIndex;
    }

    VkFormatProperties VulkanPhysicalDevice::GetPhysicalDeviceFormatProperties(
        VkFormat imageFormat) const
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_VkDevice, imageFormat, &formatProperties);
        return formatProperties;
    }
}  // namespace ENGINE_NAMESPACE
