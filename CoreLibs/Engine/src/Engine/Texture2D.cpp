#include "Texture2D.h"
#include "VulkanCheckResult.h"
#include "Buffer.h"
#include "stb_image.h"
#include "Backend.h"
#include "Api.h"
#include "Device.h"
#include "vulkan/vulkan_core.h"
namespace FooGame
{

    void CreateImage(Texture2D& image, VkExtent2D extent, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties)
    {
        auto device = Api::GetDevice();
        auto dev    = device->GetDevice();
        VkImageCreateInfo imageInfo{};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = format;
        imageInfo.tiling        = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage         = usage;
        imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        VK_CALL(vkCreateImage(dev, &imageInfo, nullptr, &image.Image));

        auto memRequirements = device->GetMemoryRequirements(image.Image);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =
            device->FindMemoryType(memRequirements.memoryTypeBits, properties);

        device->AllocateMemory(allocInfo, image.ImageMemory);

        vkBindImageMemory(dev, image.Image, image.ImageMemory, 0);
    }

    void CreateImageView(VkImage& image, VkImageView& imageView,
                         VkFormat format, VkImageAspectFlags aspectFlags)
    {
        auto device = Api::GetDevice()->GetDevice();
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image    = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format   = format;
        viewInfo.subresourceRange.aspectMask     = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        VK_CALL(vkCreateImageView(device, &viewInfo, nullptr, &imageView));
    }
    void CreateImageView(Texture2D& image, VkFormat format,
                         VkImageAspectFlags aspectFlags)
    {
        auto device = Api::GetDevice()->GetDevice();
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image    = image.Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format   = format;
        viewInfo.subresourceRange.aspectMask     = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        VK_CALL(
            vkCreateImageView(device, &viewInfo, nullptr, &image.ImageView));
    }
    void DestroyImage(VkImage& image)
    {
        auto device = Api::GetDevice()->GetDevice();
        vkDestroyImage(device, image, nullptr);
    }
    void DestroyImage(VkImage& image, VkDeviceMemory& imageMem)
    {
        auto device = Api::GetDevice()->GetDevice();
        vkDestroyImage(device, image, nullptr);
        vkFreeMemory(device, imageMem, nullptr);
    }
    void DestroyImage(Texture2D* image)
    {
        auto device = Api::GetDevice()->GetDevice();
        DestroyImage(image->Image, image->ImageMemory);
        vkDestroyImageView(device, image->ImageView, nullptr);
        if (image->Sampler)
        {
            vkDestroySampler(device, image->Sampler, nullptr);
        }
    }

    void LoadTexture(Texture2D& image, const std::string& path)
    {
        image.path     = path;
        Device* device = Api::GetDevice();
        int32_t texWidth, texHeight, texChannels;
        stbi_uc* pixels        = stbi_load(path.c_str(), &texWidth, &texHeight,
                                           &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        if (!pixels)
        {
            std::cerr << "[ERROR] "
                      << "Could not load image with path: " << path
                      << std::endl;
            return;
        }
        BufferBuilder staginfBufBuilder{};
        staginfBufBuilder.SetUsage(BufferUsage::TRANSFER_SRC)
            .SetMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            .SetInitialSize(imageSize);
        Buffer stagingBuffer = staginfBufBuilder.Build();
        stagingBuffer.Allocate();
        stagingBuffer.SetData(imageSize, pixels);
        stagingBuffer.Bind();
        stbi_image_free(pixels);

        image.width  = texWidth;
        image.height = texHeight;

        CreateImage(
            image,
            {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)},
            VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        TransitionImageLayout(&image, VK_FORMAT_R8G8B8A8_SRGB,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        stagingBuffer.CopyToImage(image);

        TransitionImageLayout(&image, VK_FORMAT_R8G8B8A8_SRGB,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        CreateImageView(image, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_ASPECT_COLOR_BIT);
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device->GetPhysicalDevice(), &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter        = VK_FILTER_LINEAR;
        samplerInfo.minFilter        = VK_FILTER_LINEAR;
        samplerInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy    = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        stagingBuffer.Release();
        VK_CALL(vkCreateSampler(device->GetDevice(), &samplerInfo, nullptr,
                                &image.Sampler));
    }
    std::shared_ptr<Texture2D> LoadTexture(const std::string& path)
    {
        Texture2D tex{};
        LoadTexture(tex, path);
        return std::make_shared<Texture2D>(std::move(tex));
    }

    void TransitionImageLayout(Texture2D* image, VkFormat format,
                               VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        auto cmd = Backend::BeginSingleTimeCommands();
        VkImageMemoryBarrier barrier{};
        barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout           = oldLayout;
        barrier.newLayout           = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image               = image->Image;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(cmd, sourceStage, destinationStage, 0, 0, nullptr,
                             0, nullptr, 1, &barrier);

        Backend::EndSingleTimeCommands(cmd);
    }
}  // namespace FooGame
