#include "Image.h"
#include "../Backend/VulkanCheckResult.h"
namespace FooGame
{
    ImageView::ImageView(Device* device, VkImage image, VkFormat format)
        : m_Image(image)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image        = image;
        createInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format       = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;

        VK_CALL(vkCreateImageView(device->GetDevice(), &createInfo, nullptr,
                                  &m_ImageView));
    }
}  // namespace FooGame
