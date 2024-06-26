#include "Device.h"
#include <vulkan/vulkan.h>
#include "pch.h"
#include "../Graphics/Api.h"
#include "../Backend/VulkanCheckResult.h"
#include "../Core/Base.h"
#include "vulkan/vulkan_core.h"
namespace FooGame
{

    VkPhysicalDeviceMemoryProperties Device::GetMemoryProperties()
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);
        return memProperties;
    }
    VkMemoryRequirements Device::GetMemoryRequirements(VkImage& image)
    {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device, image, &memRequirements);
        return memRequirements;
    }

    VkMemoryRequirements Device::GetMemoryRequirements(VkBuffer& buffer)
    {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);
        return memRequirements;
    }
    VkPhysicalDeviceProperties Device::GetPhysicalDeviceProperties()
    {
        static VkPhysicalDeviceProperties properties{};
        if (properties.deviceID == 0)
        {
            vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);
        }
        return properties;
    }

    void Device::AllocateMemory(VkMemoryAllocateInfo& allocInfo,
                                VkDeviceMemory& memory)
    {
        VK_CALL(vkAllocateMemory(m_Device, &allocInfo, nullptr, &memory));
    }

    u32 Device::FindMemoryType(u32 filter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((filter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) ==
                    properties)
            {
                return i;
            }
        }
        return 0;
    }
    void Device::WaitIdle()
    {
        vkDeviceWaitIdle(m_Device);
    }
    Device::Device(DeviceCreateInfo info)
    {
        {
            VkPhysicalDevice devices[16] = {};
            vkEnumeratePhysicalDevices(Api::GetInstance(),
                                       &m_PhysicalDeviceCount, nullptr);
            vkEnumeratePhysicalDevices(Api::GetInstance(),
                                       &m_PhysicalDeviceCount, devices);

            assert(m_PhysicalDeviceCount =
                       !0 && "Could not find physical device lol");
            VkPhysicalDeviceProperties deviceProps{};
            m_PhysicalDevice = devices[0];

            for (u32 i = 0; i < m_PhysicalDeviceCount; i++)
            {
                const auto device = devices[i];
                vkGetPhysicalDeviceProperties(device, &deviceProps);
                if (deviceProps.deviceType ==
                    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    m_PhysicalDevice = device;
                    break;
                }
            }
            VkQueueFamilyProperties queueProperties[32] = {};
            vkGetPhysicalDeviceQueueFamilyProperties(
                m_PhysicalDevice, &m_QueueFamilyCount, queueProperties);
            for (u32 i = 0; i < m_QueueFamilyCount; i++)
            {
                const auto queueF = queueProperties[i];
                if (queueF.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    m_GraphicQueueFamily = i;
                    m_PresentQueueFamily = i;  // TODO: enumarate this
                }
            }
        }
        {
            float priorities = 1.0f;
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = m_GraphicQueueFamily;
            queueCreateInfo.queueCount       = 1;
            queueCreateInfo.pQueuePriorities = &priorities;

            VkPhysicalDeviceFeatures deviceFeatures{};
            deviceFeatures.samplerAnisotropy = VK_TRUE;

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.queueCreateInfoCount = 1;
            createInfo.pQueueCreateInfos    = &queueCreateInfo;
            createInfo.pEnabledFeatures     = &deviceFeatures;
            createInfo.enabledExtensionCount =
                static_cast<u32>(info.deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = info.deviceExtensions.data();
            createInfo.enabledLayerCount =
                static_cast<u32>(info.validationLayers.size());
            createInfo.ppEnabledLayerNames = info.validationLayers.data();

            VK_CALL(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr,
                                   &m_Device));
            vkGetDeviceQueue(m_Device, m_GraphicQueueFamily, 0,
                             &m_GraphicsQueue);
            vkGetDeviceQueue(m_Device, m_PresentQueueFamily, 0,
                             &m_PresentQueue);
        }
    }
    VkSurfaceCapabilitiesKHR Device::GetSurfaceCaps(VkSurfaceKHR surface)
    {
        VkSurfaceCapabilitiesKHR caps{};
        VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice,
                                                          surface, &caps));
        return caps;
    }
    List<VkSurfaceFormatKHR> Device::GetSurfaceFormats(VkSurfaceKHR surface)
    {
        u32 formatCount = 0;
        List<VkSurfaceFormatKHR> formats;
        VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface,
                                                     &formatCount, nullptr));
        formats.resize(formatCount);
        VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(
            m_PhysicalDevice, surface, &formatCount, formats.data()));
        return formats;
    }

    List<VkPresentModeKHR> Device::GetSurfacePresentModes(VkSurfaceKHR surface)
    {
        u32 count;
        List<VkPresentModeKHR> modes{};
        VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(
            m_PhysicalDevice, surface, &count, nullptr));
        modes.reserve(count);
        VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(
            m_PhysicalDevice, surface, &count, modes.data()));
        return modes;
    }
    Device* Device::CreateDevice(const List<const char*>& layers,
                                 const List<const char*>& extensions)
    {
        return new Device({layers, extensions});
    }

}  // namespace FooGame
