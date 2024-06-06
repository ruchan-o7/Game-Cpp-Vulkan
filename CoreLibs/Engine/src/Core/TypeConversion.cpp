#include "TypeConversion.h"
#include "Types.h"
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
    VkBufferUsageFlags BuToVkBufferUsage(BufferUsage usage)
    {
        VkBufferUsageFlags usageFlag{};
        switch (usage)
        {
            case BufferUsage::TRANSFER_DST_INDEX:
            {
                usageFlag = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }
            break;
            case BufferUsage::TRANSFER_SRC:
            {
                usageFlag = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            }
            break;
            case BufferUsage::TRANSFER_DST:
            {
                usageFlag = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            break;

            case BufferUsage::TRANSFER_DST_VERTEX:
            {
                usageFlag = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            }
            break;
        }
        return usageFlag;
    }
}  // namespace ENGINE_NAMESPACE
