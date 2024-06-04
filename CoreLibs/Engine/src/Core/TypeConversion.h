#include "../Defines.h"
#include "Types.h"
#include <vulkan/vulkan.h>
namespace ENGINE_NAMESPACE
{
    ADAPTER_TYPE PhysicalDeviceTypeToAdapterType(VkPhysicalDeviceType deviceType);

    ADAPTER_VENDOR VendorIdToAdapterVendor(uint32_t vendorId);
}  // namespace ENGINE_NAMESPACE
