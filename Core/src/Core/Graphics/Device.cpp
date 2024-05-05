#include "Device.h"
#include <vulkan/vulkan_core.h>
#include <xerrc.h>
#include <stdexcept>
#include "../Backend/VulkanCheckResult.h"
#include "Core/Core/Base.h"
namespace FooGame
{
    DeviceCreateBuilder& DeviceCreateBuilder::AddExtension(
        const char* extension)
    {
        ci.deviceExtensions.push_back(extension);
        ci.deviceExtensionCount++;
        return *this;
    }
    DeviceCreateBuilder& DeviceCreateBuilder::AddLayer(const char* layer)
    {
        ci.validationLayers.push_back(layer);
        ci.validationLayersCount++;
        return *this;
    }

    Shared<Device> DeviceCreateBuilder::Build()
    {
        auto device = CreateShared<Device>();
        device->InitDevices(ci);
        return device;
    }
    Device::~Device()
    {
        vkDestroyDevice(m_Device, nullptr);
    }
    void Device::InitDevices(DeviceCreateInfo deviceCreateInfo)
    {
        {
            VkPhysicalDevice devices[16] = {};
            vkEnumeratePhysicalDevices(*deviceCreateInfo.pInstance,
                                       &m_PhysicalDeviceCount, devices);
            if (m_PhysicalDeviceCount == 0)
            {
                throw std::runtime_error("Could not find physical device lol");
            }
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

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.queueCreateInfoCount = 1;
            createInfo.pQueueCreateInfos    = &queueCreateInfo;
            createInfo.pEnabledFeatures     = &deviceFeatures;
            createInfo.enabledExtensionCount =
                deviceCreateInfo.deviceExtensionCount;
            createInfo.ppEnabledExtensionNames =
                deviceCreateInfo.deviceExtensions.data();
            createInfo.enabledLayerCount =
                deviceCreateInfo.validationLayersCount;
            createInfo.ppEnabledLayerNames =
                deviceCreateInfo.validationLayers.data();
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

}  // namespace FooGame
