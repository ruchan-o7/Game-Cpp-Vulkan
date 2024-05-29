#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>
#include <string>
namespace FooGame
{
    struct Texture2D
    {
            VkImage Image{};
            VkImageView ImageView{};
            VkDeviceMemory ImageMemory{};
            VkImageLayout ImageLayout;
            VkSampler Sampler{};
            int32_t Width, Height, MipLevels, LayerCount;
            VkDescriptorImageInfo Descriptor{};
            std::string Path = "Texture";
    };

    VkImageView CreateImageView(VkFormat format,
                                VkImageAspectFlags aspectFlags);
    void CreateImageView(Texture2D& texture, VkFormat format,
                         VkImageAspectFlags aspectFlags);

    void CreateImageView(VkImageView& imageView, VkFormat format,
                         VkImageAspectFlags aspectFlags);
    void CreateImageView(VkImage& texture, VkImageView& imageView,
                         VkFormat format, VkImageAspectFlags aspectFlags);
    VkImage CreateImage(VkExtent2D extent, VkFormat format,
                        VkImageTiling tiling, VkImageUsageFlags usage,
                        VkMemoryPropertyFlags properties);
    void CreateImage(Texture2D& texture, VkExtent2D extent, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties);
    void TransitionImageLayout(Texture2D* texture, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);
}  // namespace FooGame
