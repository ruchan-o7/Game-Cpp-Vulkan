#pragma once
#include <vulkan/vulkan.h>
#include "../Core/Base.h"
namespace FooGame
{
    struct Image
    {
            VkImage Image;
            VkImageView ImageView;
            VkDeviceMemory ImageMemory;
            u32 width, height;
    };
    void CreateImageView(Image& image, VkFormat format,
                         VkImageAspectFlags aspectFlags);

    void CreateImageView(VkImage& image, VkImageView& imageView,
                         VkFormat format, VkImageAspectFlags aspectFlags);
    void CreateImage(Image& image, VkExtent2D extent, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties);
    void DestroyImage(Image& image);
    void DestroyImage(VkImage& image);
    void DestroyImage(VkImage& image, VkDeviceMemory& imageMem);
    void LoadTexture(Image& image, const std::string& path);
    void TransitionImageLayout(Image& image, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);
}  // namespace FooGame
