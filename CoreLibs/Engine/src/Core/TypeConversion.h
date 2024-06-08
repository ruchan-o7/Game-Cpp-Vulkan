#pragma once
#include "../Defines.h"
#include "Types.h"
#include <vulkan/vulkan.h>
namespace ENGINE_NAMESPACE
{
    ADAPTER_TYPE PhysicalDeviceTypeToAdapterType(VkPhysicalDeviceType deviceType);

    ADAPTER_VENDOR VendorIdToAdapterVendor(uint32_t vendorId);
    VkBufferUsageFlags UsageToVkUsage(USAGE usage);
    // VkBufferUsageFlags BuToVkBufferUsage(BufferUsage usage);

    VkFormat TexFormatToVkFormat(TEXTURE_FORMAT format);
    VkAttachmentLoadOp LoadOpToVk(ATTACHMENT_LOAD_OP op);
    VkAttachmentStoreOp StoreOpToVk(ATTACHMENT_STORE_OP op);

    VkImageLayout RsToImageLayout(RESOURCE_STATE state);

    VkMemoryPropertyFlags BufMemFlagToVkFlag(Vulkan::BufferMemoryFlag flag);
    VkBufferUsageFlags BuffUsageToVkUsage(Vulkan::BufferUsage usage);
}  // namespace ENGINE_NAMESPACE
