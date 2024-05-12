#pragma once
#include <vulkan/vulkan.h>
#include "../Core/Base.h"
namespace FooGame
{
    struct Texture2D
    {
            VkImage Image;
            VkImageView ImageView;
            VkDeviceMemory ImageMemory;
            u32 width, height;
    };
    void CreateImageView(Texture2D& texture, VkFormat format,
                         VkImageAspectFlags aspectFlags);

    void CreateImageView(VkImage& texture, VkImageView& imageView,
                         VkFormat format, VkImageAspectFlags aspectFlags);
    void CreateImage(Texture2D& texture, VkExtent2D extent, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties);
    void DestroyImage(Texture2D& texture);
    void DestroyImage(VkImage& texture);
    void DestroyImage(VkImage& texture, VkDeviceMemory& imageMem);
    void LoadTexture(Texture2D& texture, const std::string& path);
    Shared<Texture2D> LoadTexture(const std::string& path);
    void TransitionImageLayout(Texture2D& texture, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);
}  // namespace FooGame
