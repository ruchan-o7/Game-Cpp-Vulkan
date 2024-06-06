#pragma once
#include <cstdint>
#include <vector>
#include "../Defines.h"
#include <vulkan/vulkan.h>
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

    enum BIND_FLAGS : int32_t
    {
        BIND_NONE,
        BIND_VERTEX_BUFFER,
        BIND_INDEX_BUFFER,
        BIND_UNIFORM_BUFFER,
    };
    enum USAGE : uint8_t
    {
        USAGE_GPU_ONLY,
        USAGE_DYNAMIC,
        USAGE_STAGING
    };
    enum RESOURCE_STATE : uint32_t
    {
        /// The resource state is not known to the engine and is managed by the application
        RESOURCE_STATE_UNKNOWN = 0,

        /// The resource state is known to the engine, but is undefined. A resource is typically in
        /// an undefined state right after initialization.
        RESOURCE_STATE_UNDEFINED = 1u << 0,

        /// The resource is accessed as a vertex buffer
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_VERTEX_BUFFER = 1u << 1,

        /// The resource is accessed as a constant (uniform) buffer
        /// \remarks Supported contexts: graphics, compute.
        RESOURCE_STATE_CONSTANT_BUFFER = 1u << 2,

        /// The resource is accessed as an index buffer
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_INDEX_BUFFER = 1u << 3,

        /// The resource is accessed as a render target
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_RENDER_TARGET = 1u << 4,

        /// The resource is used in a writable depth-stencil view or in clear operation
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_DEPTH_WRITE = 1u << 6,

        /// The resource is used in a read-only depth-stencil view
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_DEPTH_READ = 1u << 7,

        /// The resource is accessed from a shader
        /// \remarks Supported contexts: graphics, compute.
        RESOURCE_STATE_SHADER_RESOURCE = 1u << 8,

        /// The resource is used as the destination in a copy operation
        /// \remarks Supported contexts: graphics, compute, transfer.
        RESOURCE_STATE_COPY_DEST = 1u << 11,

        /// The resource is used as the source in a copy operation
        /// \remarks Supported contexts: graphics, compute, transfer.
        RESOURCE_STATE_COPY_SOURCE = 1u << 12,

        /// The resource is used as the destination in a resolve operation
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_RESOLVE_DEST = 1u << 13,

        /// The resource is used as the source in a resolve operation
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_RESOLVE_SOURCE = 1u << 14,

        /// The resource is used as an input attachment in a render pass subpass
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_INPUT_ATTACHMENT = 1u << 15,

        /// The resource is used for present
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_PRESENT = 1u << 16,

        /// The resource is used as vertex/index/instance buffer in an AS building operation
        /// or as an acceleration structure source in an AS copy operation.
        /// \remarks Supported contexts: graphics, compute.
        RESOURCE_STATE_BUILD_AS_READ = 1u << 17,

        /// The resource is used as the target for AS building or AS copy operations.
        /// \remarks Supported contexts: graphics, compute.
        RESOURCE_STATE_BUILD_AS_WRITE = 1u << 18,

        /// The resource state is used for read operations, but access to the resource may be slower
        /// compared to the specialized state. A transition to the COMMON state is always a pipeline
        /// stall and can often induce a cache flush and render target decompress operation.
        /// \remarks In D3D12 backend, a resource must be in COMMON state for transition between
        /// graphics/compute queue and copy queue. \remarks Supported contexts: graphics, compute,
        /// transfer.
        RESOURCE_STATE_COMMON = 1u << 20,

        /// The resource is used as the source when variable shading rate rendering.
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_SHADING_RATE = 1u << 21,

        RESOURCE_STATE_MAX_BIT = RESOURCE_STATE_SHADING_RATE,

        RESOURCE_STATE_GENERIC_READ = RESOURCE_STATE_VERTEX_BUFFER |
                                      RESOURCE_STATE_CONSTANT_BUFFER | RESOURCE_STATE_INDEX_BUFFER |
                                      RESOURCE_STATE_SHADER_RESOURCE | RESOURCE_STATE_COPY_SOURCE
    };
    enum class BufferUsage
    {
        TRANSFER_SRC,
        TRANSFER_DST,
        TRANSFER_DST_VERTEX,
        TRANSFER_DST_INDEX,
    };
}  // namespace ENGINE_NAMESPACE
