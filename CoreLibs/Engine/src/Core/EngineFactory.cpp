#include "EngineFactory.h"
#include <stdlib.h>
#include "TypeConversion.h"
#include "VulkanLogicalDevice.h"
#include <Log.h>
namespace ENGINE_NAMESPACE
{
    static constexpr uint32_t MAX_ADAPTER_QUEUES = 16;

    static constexpr uint8_t DEFAULT_QUEUE_ID = 0xFF;
    void EngineFactory::Init(const EngineCreateInfo& ci)
    {
        VulkanInstance::CreateInfo instanceCi;
        instanceCi.debugMode            = ci.debugMode;
        instanceCi.additionalLayers     = ci.additionalLayers;
        instanceCi.additionalExtensions = ci.additionalExtensions;
        instanceCi.allocCallback        = ci.allocCallback;
        instanceCi.engineVersion        = ci.engineApiVersion;
        auto instance                   = VulkanInstance::Create(instanceCi);

        VulkanPhysicalDevice::CreateInfo pCi{*instance.get(), instance->SelectPhysicalDevice(0)};
        auto pDevice = VulkanPhysicalDevice::Create(pCi);

        std::vector<const char*> deviceExts;
        if (instance->IsExtensionEnabled(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        {
            deviceExts.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }
        if (pDevice->IsExtensionSupported(VK_KHR_MAINTENANCE1_EXTENSION_NAME))
        {
            deviceExts.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        }
        else
        {
            FOO_ENGINE_WARN("{0} is not supported", VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        }
        const auto adapterInfo = GetPhysicalDeviceGraphicsAdapterInfo(*pDevice);

        auto queueProps       = pDevice->GetQueueProperties();
        uint32_t graphicQueue = pDevice->FindQueueFamily(VK_QUEUE_GRAPHICS_BIT);
        float priorities      = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = graphicQueue;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &priorities;

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.flags                = 0;
        deviceCreateInfo.enabledLayerCount    = 0;
        deviceCreateInfo.ppEnabledLayerNames  = nullptr;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos    = &queueCreateInfo;

        auto deviceFeatures = pDevice->GetFeatures();

        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

        deviceCreateInfo.ppEnabledExtensionNames = deviceExts.empty() ? nullptr : deviceExts.data();
        deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExts.size());

        auto vkAllocator = instance->GetVkAllocator();

        auto logicalDevice = VulkanLogicalDevice::Create(*pDevice, deviceCreateInfo, vkAllocator);
    }

    GraphicsAdapterInfo GetPhysicalDeviceGraphicsAdapterInfo(const VulkanPhysicalDevice& pDevice)
    {
        GraphicsAdapterInfo adapterInfo;
        const auto vkVersion       = pDevice.GetVkVersion();
        const auto& vkDeviceProps  = pDevice.GetDeviceProperties();
        const auto& vkFeatures     = pDevice.GetFeatures();
        const auto& vkDeviceLimits = vkDeviceProps.limits;
        {
            static_assert(_countof(adapterInfo.description) <= _countof(vkDeviceProps.deviceName),
                          "");
            for (size_t i = 0;
                 i < _countof(adapterInfo.description) - 1 && vkDeviceProps.deviceName[i] != 0; i++)
            {
                adapterInfo.description[i] = vkDeviceProps.deviceName[i];
            }
            adapterInfo.type       = PhysicalDeviceTypeToAdapterType(vkDeviceProps.deviceType);
            adapterInfo.vendor     = VendorIdToAdapterVendor(vkDeviceProps.vendorID);
            adapterInfo.VendorId   = vkDeviceProps.vendorID;
            adapterInfo.DeviceId   = vkDeviceProps.deviceID;
            adapterInfo.NumOutputs = 0;
        }

        return adapterInfo;
    }

}  // namespace ENGINE_NAMESPACE
