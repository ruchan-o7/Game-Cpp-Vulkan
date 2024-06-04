#pragma once
#include <cstdint>
#include <vector>
#include "../Defines.h"
#include "vulkan/vulkan_core.h"
namespace ENGINE_NAMESPACE
{
    enum ADAPTER_VENDOR_ID
    {
        ADAPTER_VENDOR_ID_AMD    = 0x01002,
        ADAPTER_VENDOR_ID_NVIDIA = 0x010DE,
        ADAPTER_VENDOR_ID_INTEL  = 0x08086,
    };
    enum ADAPTER_VENDOR : uint8_t
    {
        /// Adapter vendor is unknown
        ADAPTER_VENDOR_UNKNOWN = 0,

        /// Adapter vendor is NVidia
        ADAPTER_VENDOR_NVIDIA,

        /// Adapter vendor is AMD
        ADAPTER_VENDOR_AMD,

        /// Adapter vendor is Intel
        ADAPTER_VENDOR_INTEL,

        ADAPTER_VENDOR_LAST = ADAPTER_VENDOR_INTEL
    };

    enum ADAPTER_TYPE : uint8_t
    {
        ADAPTER_TYPE_UNKNOWN = 0,
        ADAPTER_TYPE_SOFTWARE,
        ADAPTER_TYPE_INTEGRATED,
        ADAPTER_TYPE_DISCRETE,
        ADAPTER_TYPE_COUNT
    };

    enum QUEUE_PRIORITY : uint8_t
    {
        QUEUE_PRIORITY_UNKNOWN = 0,

        /// Vulkan backend:     VK_QUEUE_GLOBAL_PRIORITY_LOW_EXT
        QUEUE_PRIORITY_LOW,

        /// Vulkan backend:     VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXT
        QUEUE_PRIORITY_MEDIUM,

        /// Vulkan backend:     VK_QUEUE_GLOBAL_PRIORITY_HIGH_EXT
        QUEUE_PRIORITY_HIGH,

        /// Additional system privileges required to use this priority, read documentation for
        /// specific platform. Vulkan backend:     VK_QUEUE_GLOBAL_PRIORITY_REALTIME_EXT Direct3D12
        QUEUE_PRIORITY_REALTIME,

        QUEUE_PRIORITY_LAST = QUEUE_PRIORITY_REALTIME
    };
    struct EngineCreateInfo
    {
            uint32_t engineApiVersion = VK_MAKE_VERSION(0, 1, 0);
            uint32_t adapterId        = 0;

            bool enableValidation = true;
            std::vector<const char*> additionalExtensions;
            std::vector<const char*> additionalLayers;

            VkAllocationCallbacks* allocCallback = nullptr;
            bool debugMode                       = true;
    };
    struct GraphicsAdapterInfo
    {
            char description[128] = {};
            ADAPTER_TYPE type;
            ADAPTER_VENDOR vendor;
            uint32_t VendorId   = (0);
            uint32_t DeviceId   = (0);
            uint32_t NumOutputs = (0);
    };
}  // namespace ENGINE_NAMESPACE
