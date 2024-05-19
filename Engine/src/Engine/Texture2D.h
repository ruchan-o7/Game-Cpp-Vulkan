#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <string>
namespace FooGame
{

    struct Texture2D
    {
            VkImage Image;
            VkImageView ImageView;
            VkDeviceMemory ImageMemory;
            VkSampler Sampler;
            uint32_t width, height;
            std::string path;
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
    void DestroyImage(Texture2D* texture);
    void DestroyImage(VkImage& texture);
    void DestroyImage(VkImage& texture, VkDeviceMemory& imageMem);
    void LoadTexture(Texture2D* texture, const std::string& path);
    [[nodiscard]] std::shared_ptr<Texture2D> LoadTexture(
        const std::string& path);
    void TransitionImageLayout(Texture2D* texture, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);
}  // namespace FooGame
