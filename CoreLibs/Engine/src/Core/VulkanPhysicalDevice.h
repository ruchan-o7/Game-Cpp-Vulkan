#pragma once
#include <memory>
#include "VulkanInstance.h"
#include "Utils/IndexWrapper.h"
namespace ENGINE_NAMESPACE
{

    class VulkanPhysicalDevice
    {
        public:
            struct CreateInfo
            {
                    const VulkanInstance& instance;
                    const VkPhysicalDevice& vkDevice;
                    bool printExtensions = true;
            };
            DELETE_COPY_MOVE(VulkanPhysicalDevice);
            static std::unique_ptr<VulkanPhysicalDevice> Create(
                const VulkanPhysicalDevice::CreateInfo& ci);

            HardwareQueueIndex FindQueueFamily(VkQueueFlags queueFlag) const;
            bool IsExtensionSupported(const char* ExtensionName) const;

            static constexpr uint32_t InvalidMemoryTypeIndex = ~uint32_t{0};

            uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags props) const;

            const VkPhysicalDevice& GetVkDeviceHandle() const { return m_VkDevice; }
            uint32_t GetVkVersion() const { return m_VkVersion; }
            const VkPhysicalDeviceProperties& GetDeviceProperties() const { return m_Properties; };
            const VkPhysicalDeviceFeatures& GetFeatures() const { return m_Features; };
            const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const
            {
                return m_MemoryProps;
            };

            const std::vector<VkQueueFamilyProperties>& GetQueueProperties() const
            {
                return m_QueueFamilyProperties;
            }

            VkFormatProperties GetPhysicalDeviceFormatProperties(VkFormat imageFormat) const;

        private:
            VulkanPhysicalDevice(const VulkanPhysicalDevice::CreateInfo& ci);

        private:
            const VkPhysicalDevice m_VkDevice;
            VkPhysicalDeviceProperties m_Properties;
            VkPhysicalDeviceFeatures m_Features;
            VkPhysicalDeviceMemoryProperties m_MemoryProps;
            std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
            std::vector<VkExtensionProperties> m_SupportedExtensions;
            uint32_t m_VkVersion;
    };

}  // namespace ENGINE_NAMESPACE
