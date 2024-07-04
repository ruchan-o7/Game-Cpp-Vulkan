#include "TypeConversion.h"
#include <cassert>
#include "Types.h"
#include "vulkan/vulkan_core.h"
#include <Log.h>
namespace ENGINE_NAMESPACE
{

    ADAPTER_TYPE PhysicalDeviceTypeToAdapterType(VkPhysicalDeviceType deviceType)
    {
        switch (deviceType)
        {
                // clang-format off
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return ADAPTER_TYPE_INTEGRATED;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return ADAPTER_TYPE_DISCRETE;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:            return ADAPTER_TYPE_SOFTWARE;
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        default:                                     return ADAPTER_TYPE_UNKNOWN;
                // clang-format on
        }
    }

    ADAPTER_VENDOR VendorIdToAdapterVendor(uint32_t vendorId)
    {
        switch (vendorId)
        {
#define VENDOR_ID_TO_VENDOR(Name)  \
    case ADAPTER_VENDOR_ID_##Name: \
        return ADAPTER_VENDOR_##Name

            VENDOR_ID_TO_VENDOR(AMD);
            VENDOR_ID_TO_VENDOR(NVIDIA);
            VENDOR_ID_TO_VENDOR(INTEL);
#undef VENDOR_ID_TO_VENDOR

            default:
                return ADAPTER_VENDOR_UNKNOWN;
        }
    }
    class TexFormatToVkFormatMapper
    {
        public:
            TexFormatToVkFormatMapper()
            {
                // clang-format off
            m_FmtToVkFmtMap[TEX_FORMAT_UNKNOWN] = VK_FORMAT_UNDEFINED;

            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA32_TYPELESS] = VK_FORMAT_R32G32B32A32_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA32_FLOAT]    = VK_FORMAT_R32G32B32A32_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA32_UINT]     = VK_FORMAT_R32G32B32A32_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA32_SINT]     = VK_FORMAT_R32G32B32A32_SINT;
            //
            // m_FmtToVkFmtMap[TEX_FORMAT_RGB32_TYPELESS] = VK_FORMAT_R32G32B32_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGB32_FLOAT]    = VK_FORMAT_R32G32B32_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGB32_UINT]     = VK_FORMAT_R32G32B32_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGB32_SINT]     = VK_FORMAT_R32G32B32_SINT;
            //
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA16_TYPELESS] = VK_FORMAT_R16G16B16A16_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA16_FLOAT]    = VK_FORMAT_R16G16B16A16_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA16_UNORM]    = VK_FORMAT_R16G16B16A16_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA16_UINT]     = VK_FORMAT_R16G16B16A16_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA16_SNORM]    = VK_FORMAT_R16G16B16A16_SNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA16_SINT]     = VK_FORMAT_R16G16B16A16_SINT;
            //
            // m_FmtToVkFmtMap[TEX_FORMAT_RG32_TYPELESS] = VK_FORMAT_R32G32_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG32_FLOAT]    = VK_FORMAT_R32G32_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG32_UINT]     = VK_FORMAT_R32G32_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG32_SINT]     = VK_FORMAT_R32G32_SINT;
            //
            // m_FmtToVkFmtMap[TEX_FORMAT_R32G8X24_TYPELESS]        = VK_FORMAT_D32_SFLOAT_S8_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_D32_FLOAT_S8X24_UINT]     = VK_FORMAT_D32_SFLOAT_S8_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS] = VK_FORMAT_D32_SFLOAT_S8_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_X32_TYPELESS_G8X24_UINT]  = VK_FORMAT_UNDEFINED;
            //
            // m_FmtToVkFmtMap[TEX_FORMAT_RGB10A2_TYPELESS] = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGB10A2_UNORM]    = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGB10A2_UINT]     = VK_FORMAT_A2R10G10B10_UINT_PACK32;
            // m_FmtToVkFmtMap[TEX_FORMAT_R11G11B10_FLOAT]  = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            //
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA8_TYPELESS]   = VK_FORMAT_R8G8B8A8_UNORM;
            m_FmtToVkFmtMap[TEX_FORMAT_RGBA8_UNORM]      = VK_FORMAT_R8G8B8A8_UNORM;
            m_FmtToVkFmtMap[TEX_FORMAT_RGBA8_UNORM_SRGB] = VK_FORMAT_R8G8B8A8_SRGB;
            m_FmtToVkFmtMap[TEX_FORMAT_RGBA8_UINT]       = VK_FORMAT_R8G8B8A8_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA8_SNORM]      = VK_FORMAT_R8G8B8A8_SNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_RGBA8_SINT]       = VK_FORMAT_R8G8B8A8_SINT;

            // m_FmtToVkFmtMap[TEX_FORMAT_RG16_TYPELESS] = VK_FORMAT_R16G16_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG16_FLOAT]    = VK_FORMAT_R16G16_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG16_UNORM]    = VK_FORMAT_R16G16_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG16_UINT]     = VK_FORMAT_R16G16_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG16_SNORM]    = VK_FORMAT_R16G16_SNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG16_SINT]     = VK_FORMAT_R16G16_SINT;
            //
            // m_FmtToVkFmtMap[TEX_FORMAT_R32_TYPELESS] = VK_FORMAT_R32_SFLOAT;
            m_FmtToVkFmtMap[TEX_FORMAT_D32_FLOAT]    = VK_FORMAT_D32_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_R32_FLOAT]    = VK_FORMAT_R32_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_R32_UINT]     = VK_FORMAT_R32_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_R32_SINT]     = VK_FORMAT_R32_SINT;

            // m_FmtToVkFmtMap[TEX_FORMAT_R24G8_TYPELESS]        = VK_FORMAT_D24_UNORM_S8_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_D24_UNORM_S8_UINT]     = VK_FORMAT_D24_UNORM_S8_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_R24_UNORM_X8_TYPELESS] = VK_FORMAT_D24_UNORM_S8_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_X24_TYPELESS_G8_UINT]  = VK_FORMAT_UNDEFINED;
            //
            // m_FmtToVkFmtMap[TEX_FORMAT_RG8_TYPELESS] = VK_FORMAT_R8G8_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG8_UNORM]    = VK_FORMAT_R8G8_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG8_UINT]     = VK_FORMAT_R8G8_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG8_SNORM]    = VK_FORMAT_R8G8_SNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG8_SINT]     = VK_FORMAT_R8G8_SINT;
            //
            // m_FmtToVkFmtMap[TEX_FORMAT_R16_TYPELESS] = VK_FORMAT_R16_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_R16_FLOAT]    = VK_FORMAT_R16_SFLOAT;
            // m_FmtToVkFmtMap[TEX_FORMAT_D16_UNORM]    = VK_FORMAT_D16_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_R16_UNORM]    = VK_FORMAT_R16_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_R16_UINT]     = VK_FORMAT_R16_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_R16_SNORM]    = VK_FORMAT_R16_SNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_R16_SINT]     = VK_FORMAT_R16_SINT;

            // m_FmtToVkFmtMap[TEX_FORMAT_R8_TYPELESS] = VK_FORMAT_R8_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_R8_UNORM]    = VK_FORMAT_R8_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_R8_UINT]     = VK_FORMAT_R8_UINT;
            // m_FmtToVkFmtMap[TEX_FORMAT_R8_SNORM]    = VK_FORMAT_R8_SNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_R8_SINT]     = VK_FORMAT_R8_SINT;
            // To get the same behaviour as expected for TEX_FORMAT_A8_UNORM we
            // swizzle the components appropriately using the VkImageViewCreateInfo struct
            // m_FmtToVkFmtMap[TEX_FORMAT_A8_UNORM]    = VK_FORMAT_R8_UNORM;
            //
            // m_FmtToVkFmtMap[TEX_FORMAT_R1_UNORM]    = VK_FORMAT_UNDEFINED;

            // m_FmtToVkFmtMap[TEX_FORMAT_RGB9E5_SHAREDEXP] = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
            // m_FmtToVkFmtMap[TEX_FORMAT_RG8_B8G8_UNORM]   = VK_FORMAT_UNDEFINED;
            // m_FmtToVkFmtMap[TEX_FORMAT_G8R8_G8B8_UNORM]  = VK_FORMAT_UNDEFINED;

            // http://www.g-truc.net/post-0335.html
            // http://renderingpipeline.com/2012/07/texture-compression/
            // m_FmtToVkFmtMap[TEX_FORMAT_BC1_TYPELESS]   = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC1_UNORM]      = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC1_UNORM_SRGB] = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC2_TYPELESS]   = VK_FORMAT_BC2_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC2_UNORM]      = VK_FORMAT_BC2_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC2_UNORM_SRGB] = VK_FORMAT_BC2_SRGB_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC3_TYPELESS]   = VK_FORMAT_BC3_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC3_UNORM]      = VK_FORMAT_BC3_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC3_UNORM_SRGB] = VK_FORMAT_BC3_SRGB_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC4_TYPELESS]   = VK_FORMAT_BC4_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC4_UNORM]      = VK_FORMAT_BC4_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC4_SNORM]      = VK_FORMAT_BC4_SNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC5_TYPELESS]   = VK_FORMAT_BC5_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC5_UNORM]      = VK_FORMAT_BC5_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC5_SNORM]      = VK_FORMAT_BC5_SNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_B5G6R5_UNORM]   = VK_FORMAT_B5G6R5_UNORM_PACK16;
            // m_FmtToVkFmtMap[TEX_FORMAT_B5G5R5A1_UNORM] = VK_FORMAT_B5G5R5A1_UNORM_PACK16;
            // m_FmtToVkFmtMap[TEX_FORMAT_BGRA8_UNORM]    = VK_FORMAT_B8G8R8A8_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_BGRX8_UNORM]    = VK_FORMAT_B8G8R8A8_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_R10G10B10_XR_BIAS_A2_UNORM] = VK_FORMAT_UNDEFINED;
            // m_FmtToVkFmtMap[TEX_FORMAT_BGRA8_TYPELESS]   = VK_FORMAT_B8G8R8A8_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_BGRA8_UNORM_SRGB] = VK_FORMAT_B8G8R8A8_SRGB;
            // m_FmtToVkFmtMap[TEX_FORMAT_BGRX8_TYPELESS]   = VK_FORMAT_B8G8R8A8_UNORM;
            // m_FmtToVkFmtMap[TEX_FORMAT_BGRX8_UNORM_SRGB] = VK_FORMAT_B8G8R8A8_SRGB;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC6H_TYPELESS] = VK_FORMAT_BC6H_UFLOAT_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC6H_UF16]     = VK_FORMAT_BC6H_UFLOAT_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC6H_SF16]     = VK_FORMAT_BC6H_SFLOAT_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC7_TYPELESS]   = VK_FORMAT_BC7_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC7_UNORM]      = VK_FORMAT_BC7_UNORM_BLOCK;
            // m_FmtToVkFmtMap[TEX_FORMAT_BC7_UNORM_SRGB] = VK_FORMAT_BC7_SRGB_BLOCK;
                // clang-format on
            }

            VkFormat operator[](TEXTURE_FORMAT TexFmt) const
            {
                assert(TexFmt < _countof(m_FmtToVkFmtMap));
                return m_FmtToVkFmtMap[TexFmt];
            }

        private:
            VkFormat m_FmtToVkFmtMap[TEX_FORMAT_NUM_FORMATS] = {};
    };
    VkFormat TexFormatToVkFormat(TEXTURE_FORMAT format)
    {
        static const TexFormatToVkFormatMapper FmtMapper;
        return FmtMapper[format];
    }

    VkAttachmentLoadOp LoadOpToVk(ATTACHMENT_LOAD_OP op)
    {
        return static_cast<VkAttachmentLoadOp>(op);
    }
    VkAttachmentStoreOp StoreOpToVk(ATTACHMENT_STORE_OP op)
    {
        return static_cast<VkAttachmentStoreOp>(op);
    }

    VkImageLayout RsToImageLayout(RESOURCE_STATE state)
    {
        if (state == RESOURCE_STATE_UNKNOWN)
        {
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }
        // Currently not used:
        // VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
        // VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
        // VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV
        // VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR =
        // VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        // VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR =
        // VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,

        static_assert(RESOURCE_STATE_MAX_BIT == (1u << 21),
                      "This function must be updated to handle new resource state flag");
        assert((state & (state - 1)) == 0 && "Only single bit must be set");
        switch (state)
        {
                // clang-format off
            case RESOURCE_STATE_UNDEFINED:         return VK_IMAGE_LAYOUT_UNDEFINED;
            case RESOURCE_STATE_VERTEX_BUFFER:     assert(0 && "Invalid resource state"); return VK_IMAGE_LAYOUT_UNDEFINED;
            case RESOURCE_STATE_CONSTANT_BUFFER:   assert(0 && "Invalid resource state"); return VK_IMAGE_LAYOUT_UNDEFINED;
            case RESOURCE_STATE_INDEX_BUFFER:      assert(0 && "Invalid resource state"); return VK_IMAGE_LAYOUT_UNDEFINED;
            case RESOURCE_STATE_RENDER_TARGET:     return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case RESOURCE_STATE_UNORDERED_ACCESS:  return VK_IMAGE_LAYOUT_GENERAL;
            case RESOURCE_STATE_DEPTH_WRITE:       return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            case RESOURCE_STATE_DEPTH_READ:        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            case RESOURCE_STATE_SHADER_RESOURCE:   return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case RESOURCE_STATE_STREAM_OUT:        assert(0 && "Invalid resource state"); return VK_IMAGE_LAYOUT_UNDEFINED;
            case RESOURCE_STATE_INDIRECT_ARGUMENT: assert(0 && "Invalid resource state"); return VK_IMAGE_LAYOUT_UNDEFINED;
            case RESOURCE_STATE_COPY_DEST:         return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case RESOURCE_STATE_COPY_SOURCE:       return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case RESOURCE_STATE_RESOLVE_DEST:      return   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; // TODO CHECK THIs
            case RESOURCE_STATE_RESOLVE_SOURCE:    return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case RESOURCE_STATE_INPUT_ATTACHMENT:  return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case RESOURCE_STATE_PRESENT:           return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            case RESOURCE_STATE_BUILD_AS_READ:     assert(0 && "Invalid resource state"); return VK_IMAGE_LAYOUT_UNDEFINED;
            case RESOURCE_STATE_BUILD_AS_WRITE:    assert(0 && "Invalid resource state"); return VK_IMAGE_LAYOUT_UNDEFINED;
            case RESOURCE_STATE_RAY_TRACING:       assert(0 && "Invalid resource state"); return VK_IMAGE_LAYOUT_UNDEFINED;
            case RESOURCE_STATE_COMMON:            return VK_IMAGE_LAYOUT_GENERAL;
            case RESOURCE_STATE_SHADING_RATE:      return   VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;//TODO Check this also
                // clang-format on

            default:
                FOO_ENGINE_ERROR("Unexpected resource state flag ({0})", static_cast<int>(state));
                return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }

    // VkBufferUsageFlags BuffUsageToVkUsage(Vulkan::BufferUsage usage)
    // {
    //     switch (usage)
    //     {
    //         case Vulkan::BufferUsage::VERTEX:
    //             return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    //         case Vulkan::BufferUsage::INDEX:
    //             return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    //         case Vulkan::BufferUsage::UNIFORM:
    //             return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    //             ;
    //         case Vulkan::BufferUsage::TRANSFER_DESTINATION:
    //             return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    //         case Vulkan::BufferUsage::TRANSFER_SOURCE:
    //             return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    //             break;
    //     }
    // }
}  // namespace ENGINE_NAMESPACE
