#pragma once
#include <cstdint>
#include <vector>
#include "../Defines.h"
#include <vulkan/vulkan.h>
#include <memory>
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
        ADAPTER_VENDOR_UNKNOWN = 0,
        ADAPTER_VENDOR_NVIDIA,
        ADAPTER_VENDOR_AMD,
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
        QUEUE_PRIORITY_LOW,
        QUEUE_PRIORITY_MEDIUM,
        QUEUE_PRIORITY_HIGH,

        /// Additional system privileges required to use this priority, read documentation for
        /// specific platform. Vulkan backend:     VK_QUEUE_GLOBAL_PRIORITY_REALTIME_EXT Direct3D12
        QUEUE_PRIORITY_REALTIME,
        QUEUE_PRIORITY_LAST = QUEUE_PRIORITY_REALTIME
    };

    struct DeviceContextDesc
    {
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
            uint32_t numOfContext                = 1;
            const DeviceContextDesc* pDevCtxInfo = nullptr;
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

        /// The resource is used for unordered access
        /// \remarks Supported contexts: graphics, compute.
        RESOURCE_STATE_UNORDERED_ACCESS = 1u << 5,

        /// The resource is used in a writable depth-stencil view or in clear operation
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_DEPTH_WRITE = 1u << 6,

        /// The resource is used in a read-only depth-stencil view
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_DEPTH_READ = 1u << 7,

        /// The resource is accessed from a shader
        /// \remarks Supported contexts: graphics, compute.
        RESOURCE_STATE_SHADER_RESOURCE = 1u << 8,

        /// The resource is used as the destination for stream output
        RESOURCE_STATE_STREAM_OUT = 1u << 9,

        /// The resource is used as an indirect draw/dispatch arguments buffer
        /// \remarks Supported contexts: graphics, compute.
        RESOURCE_STATE_INDIRECT_ARGUMENT = 1u << 10,

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

        /// The resource is used as a top-level AS shader resource in a trace rays operation.
        /// \remarks Supported contexts: graphics, compute.
        RESOURCE_STATE_RAY_TRACING = 1u << 19,

        /// The resource state is used for read operations, but access to the resource may be slower
        /// compared to the specialized state.
        /// A transition to the COMMON state is always a pipeline stall and can often induce a cache
        /// flush and render target decompress operation.
        /// \remarks In D3D12 backend, a resource must be in COMMON state for transition between
        /// graphics/compute queue and copy queue.
        /// \remarks Supported contexts: graphics, compute, transfer.
        RESOURCE_STATE_COMMON = 1u << 20,

        /// The resource is used as the source when variable shading rate rendering.
        /// \remarks Supported contexts: graphics.
        RESOURCE_STATE_SHADING_RATE = 1u << 21,

        RESOURCE_STATE_MAX_BIT = RESOURCE_STATE_SHADING_RATE,

        RESOURCE_STATE_GENERIC_READ = RESOURCE_STATE_VERTEX_BUFFER |
                                      RESOURCE_STATE_CONSTANT_BUFFER | RESOURCE_STATE_INDEX_BUFFER |
                                      RESOURCE_STATE_SHADER_RESOURCE |
                                      RESOURCE_STATE_INDIRECT_ARGUMENT | RESOURCE_STATE_COPY_SOURCE
    };
    // enum class BufferUsage
    // {
    //     TRANSFER_SRC,
    //     TRANSFER_DST,
    //     TRANSFER_DST_VERTEX,
    //     TRANSFER_DST_INDEX,
    // };
    class VulkanLogicalDevice;
    enum class TextureType
    {
        Texture2D,
        Texture3D,
    };
    enum class TextureFormat  // image format
    {
        Unknown,
        Png,
        Jpeg_Jpg,
    };
    struct TextureDescription
    {
            const char* Name = "Default Texture Name";
            uint32_t Width, Height;
            TextureType TextureType     = TextureType::Texture2D;
            TextureFormat TextureFormat = TextureFormat::Unknown;
            uint32_t MipLevels          = 1;
            uint32_t SampleCount        = 1;
            VkImageLayout ImageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    };

    struct TextureData
    {
            void* Data;
            size_t Size;
            std::shared_ptr<VulkanLogicalDevice> LogicalDevice;
    };
    enum TEXTURE_FORMAT : uint16_t
    {
        TEX_FORMAT_UNKNOWN = 0,
        TEX_FORMAT_RGBA8_UNORM,
        TEX_FORMAT_RGBA8_UNORM_SRGB,
        TEX_FORMAT_RGBA8_UINT,
        TEX_FORMAT_D32_FLOAT,
        TEX_FORMAT_NUM_FORMATS,
    };
    enum ATTACHMENT_LOAD_OP : uint8_t
    {
        ATTACHMENT_LOAD_OP_LOAD,
        ATTACHMENT_LOAD_OP_CLEAR,
        ATTACHMENT_LOAD_OP_DONT_CARE,
    };
    enum ATTACHMENT_STORE_OP : uint8_t
    {
        ATTACHMENT_STORE_OP_STORE,
        ATTACHMENT_STORE_OP_DONT_CARE,
    };
    enum PIPELINE_STAGE_FLAGS : uint32_t
    {
        PIPELINE_STAGE_FLAG_TOP_OF_PIPE                      = 0x00000001,
        PIPELINE_STAGE_FLAG_DRAW_INDIRECT                    = 0x00000002,
        PIPELINE_STAGE_FLAG_VERTEX_INPUT                     = 0x00000004,
        PIPELINE_STAGE_FLAG_VERTEX_SHADER                    = 0x00000008,
        PIPELINE_STAGE_FLAG_TESSELLATION_CONTROL_SHADER      = 0x00000010,
        PIPELINE_STAGE_FLAG_TESSELLATION_EVALUATION_SHADER   = 0x00000020,
        PIPELINE_STAGE_FLAG_GEOMETRY_SHADER                  = 0x00000040,
        PIPELINE_STAGE_FLAG_FRAGMENT_SHADER                  = 0x00000080,
        PIPELINE_STAGE_FLAG_EARLY_FRAGMENT_TESTS             = 0x00000100,
        PIPELINE_STAGE_FLAG_LATE_FRAGMENT_TESTS              = 0x00000200,
        PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT          = 0x00000400,
        PIPELINE_STAGE_FLAG_COMPUTE_SHADER                   = 0x00000800,
        PIPELINE_STAGE_FLAG_TRANSFER                         = 0x00001000,
        PIPELINE_STAGE_FLAG_BOTTOM_OF_PIPE                   = 0x00002000,
        PIPELINE_STAGE_FLAG_HOST                             = 0x00004000,
        PIPELINE_STAGE_FLAG_ALL_GRAPHICS                     = 0x00008000,
        PIPELINE_STAGE_FLAG_ALL_COMMANDS                     = 0x00010000,
        PIPELINE_STAGE_FLAG_NONE                             = 0,
        PIPELINE_STAGE_FLAG_TRANSFORM_FEEDBACK_BIT_EXT       = 0x01000000,
        PIPELINE_STAGE_FLAG_CONDITIONAL_RENDERING_BIT_EXT    = 0x00040000,
        PIPELINE_STAGE_FLAG_ACCELERATION_STRUCTURE_BUILD     = 0x02000000,
        PIPELINE_STAGE_FLAG_RAY_TRACING_SHADER               = 0x00200000,
        PIPELINE_STAGE_FLAG_FRAGMENT_DENSITY_PROCESS_BIT_EXT = 0x00800000,
        PIPELINE_STAGE_FLAG_FRAGMENT_SHADING_RATE_ATTACHMENT = 0x00400000,
        PIPELINE_STAGE_FLAG_COMMAND_PREPROCESS_BIT_NV        = 0x00020000,
        PIPELINE_STAGE_FLAG_TASK_SHADER                      = 0x00080000,
        PIPELINE_STAGE_FLAG_MESH_SHADER                      = 0x00100000,
        PIPELINE_STAGE_FLAG_SHADING_RATE_IMAGE_BIT_NV =
            PIPELINE_STAGE_FLAG_FRAGMENT_SHADING_RATE_ATTACHMENT,
        PIPELINE_STAGE_FLAG_RAY_TRACING_SHADER_BIT_NV = PIPELINE_STAGE_FLAG_RAY_TRACING_SHADER,
        PIPELINE_STAGE_FLAG_ACCELERATION_STRUCTURE_BUILD_BIT_NV =
            PIPELINE_STAGE_FLAG_ACCELERATION_STRUCTURE_BUILD,
        PIPELINE_STAGE_FLAG_TASK_SHADER_BIT_NV = PIPELINE_STAGE_FLAG_TASK_SHADER,
        PIPELINE_STAGE_FLAG_MESH_SHADER_BIT_NV = PIPELINE_STAGE_FLAG_MESH_SHADER,
        PIPELINE_STAGE_FLAG_NONE_KHR           = PIPELINE_STAGE_FLAG_NONE,
        PIPELINE_STAGE_FLAG_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };
    enum ACCESS_FLAGS : uint32_t
    {
        ACCESS_FLAG_INDIRECT_COMMAND_READ                         = 0x00000001,
        ACCESS_FLAG_INDEX_READ                                    = 0x00000002,
        ACCESS_FLAG_VERTEX_ATTRIBUTE_READ                         = 0x00000004,
        ACCESS_FLAG_UNIFORM_READ                                  = 0x00000008,
        ACCESS_FLAG_INPUT_ATTACHMENT_READ                         = 0x00000010,
        ACCESS_FLAG_SHADER_READ                                   = 0x00000020,
        ACCESS_FLAG_SHADER_WRITE                                  = 0x00000040,
        ACCESS_FLAG_COLOR_ATTACHMENT_READ                         = 0x00000080,
        ACCESS_FLAG_COLOR_ATTACHMENT_WRITE                        = 0x00000100,
        ACCESS_FLAG_DEPTH_STENCIL_ATTACHMENT_READ                 = 0x00000200,
        ACCESS_FLAG_DEPTH_STENCIL_ATTACHMENT_WRITE                = 0x00000400,
        ACCESS_FLAG_TRANSFER_READ                                 = 0x00000800,
        ACCESS_FLAG_TRANSFER_WRITE                                = 0x00001000,
        ACCESS_FLAG_HOST_READ                                     = 0x00002000,
        ACCESS_FLAG_HOST_WRITE                                    = 0x00004000,
        ACCESS_FLAG_MEMORY_READ                                   = 0x00008000,
        ACCESS_FLAG_MEMORY_WRITE                                  = 0x00010000,
        ACCESS_FLAG_NONE                                          = 0,
        ACCESS_FLAG_TRANSFORM_FEEDBACK_WRITE_BIT_EXT              = 0x02000000,
        ACCESS_FLAG_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT       = 0x04000000,
        ACCESS_FLAG_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT      = 0x08000000,
        ACCESS_FLAG_CONDITIONAL_RENDERING_READ_BIT_EXT            = 0x00100000,
        ACCESS_FLAG_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT     = 0x00080000,
        ACCESS_FLAG_ACCELERATION_STRUCTURE_READ_BIT_KHR           = 0x00200000,
        ACCESS_FLAG_ACCELERATION_STRUCTURE_WRITE_BIT_KHR          = 0x00400000,
        ACCESS_FLAG_FRAGMENT_DENSITY_MAP_READ_BIT_EXT             = 0x01000000,
        ACCESS_FLAG_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR = 0x00800000,
        ACCESS_FLAG_COMMAND_PREPROCESS_READ_BIT_NV                = 0x00020000,
        ACCESS_FLAG_COMMAND_PREPROCESS_WRITE_BIT_NV               = 0x00040000,
        ACCESS_FLAG_SHADING_RATE_IMAGE_READ_BIT_NV =
            ACCESS_FLAG_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR,
        ACCESS_FLAG_ACCELERATION_STRUCTURE_READ_BIT_NV =
            ACCESS_FLAG_ACCELERATION_STRUCTURE_READ_BIT_KHR,
        ACCESS_FLAG_ACCELERATION_STRUCTURE_WRITE_BIT_NV =
            ACCESS_FLAG_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
        ACCESS_FLAG_NONE_KHR           = ACCESS_FLAG_NONE,
        ACCESS_FLAG_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };
    struct RenderPassAttachmentDesc
    {
            TEXTURE_FORMAT Format = TEX_FORMAT_UNKNOWN;
            uint8_t SampleCount;

            ATTACHMENT_LOAD_OP LoadOp   = ATTACHMENT_LOAD_OP_LOAD;
            ATTACHMENT_STORE_OP StoreOp = ATTACHMENT_STORE_OP_STORE;

            ATTACHMENT_LOAD_OP StencilLoadOp   = ATTACHMENT_LOAD_OP_LOAD;
            ATTACHMENT_STORE_OP StencilStoreOp = ATTACHMENT_STORE_OP_STORE;

            RESOURCE_STATE InitialLayout = RESOURCE_STATE_UNKNOWN;
            RESOURCE_STATE FinalLayout   = RESOURCE_STATE_UNKNOWN;
    };
    struct AttachmentReference
    {
            uint32_t AttachmentIndex = 0;
            RESOURCE_STATE State     = RESOURCE_STATE_UNKNOWN;
    };
    struct SubpassDesc
    {
            uint32_t AttachmentCount                           = 1;
            const AttachmentReference* pColorAttachment        = nullptr;
            const AttachmentReference* pDepthStencilAttachment = nullptr;
    };
    struct SubpassDependencyDesc
    {
            uint32_t SrcSubpass = 0;
            uint32_t DstSubpass = 0;

            PIPELINE_STAGE_FLAGS SrcStageMask = PIPELINE_STAGE_FLAG_NONE;
            PIPELINE_STAGE_FLAGS DstStageMask = PIPELINE_STAGE_FLAG_NONE;

            ACCESS_FLAGS SrcAccessMask = ACCESS_FLAG_NONE;
            ACCESS_FLAGS DstAccessMask = ACCESS_FLAG_NONE;
    };
    struct RenderPassDesc
    {
            uint32_t AttachmentCount                     = 0;
            const RenderPassAttachmentDesc* pAttachments = nullptr;

            uint32_t SubpassCount           = 0;
            const SubpassDesc* pSubpassDesc = nullptr;

            uint32_t DependencyCount           = 0;
            const SubpassDependencyDesc* pDeps = nullptr;
    };
    namespace Vulkan
    {
        enum class BufferUsage
        {
            Vertex               = 1,
            Index                = 2,
            Uniform              = 4,
            TransferDestionation = 8,
            TransferSource       = 10,
        };
        enum class BufferMemoryFlag
        {
            CpuVisible,
            GpuOnly,
        };
    };  // namespace Vulkan

}  // namespace ENGINE_NAMESPACE
