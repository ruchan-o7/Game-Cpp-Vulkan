#pragma once
#include <vulkan/vulkan.h>
#include "../Graphics/Device.h"
namespace FooGame
{
    class Image
    {
        public:
            Image() = default;

        private:
            VkImage m_Image;
    };
    class ImageView
    {
        public:
            ImageView(Device* device, VkImage image, VkFormat format);

        private:
            VkImage m_Image;
            VkImageView m_ImageView;
    };
}  // namespace FooGame
