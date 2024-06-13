#pragma once
#include <memory>
#include "VulkanLogicalDevice.h"
#include "Types.h"
#include "Utils/VulkanObjectWrapper.h"
namespace ENGINE_NAMESPACE
{
    class VulkanImage
    {
            DELETE_COPY(VulkanImage);

        public:
            struct ImageDesc
            {
                    VkExtent2D Extent;
                    VkFormat Format;
                    VkImageTiling Tiling;
                    VkImageUsageFlags Usage;
                    VkMemoryPropertyFlags MemoryPropertiesFlags;
                    class RenderDevice* pRenderDevice;
            };
            VulkanImage(const ImageDesc& desc);
            VkImage GetImageHandle() const { return m_Image; }

        private:
            ImageDesc m_Desc;
            ImageWrapper m_Image;
            DeviceMemoryWrapper m_ImageMemory;
    };
    class VulkanImageView
    {
            DELETE_COPY(VulkanImageView);

        public:
            struct ImageViewDesc
            {
                    VkImage Image;
                    VkFormat Format;
                    VkImageAspectFlagBits Aspects;
                    std::shared_ptr<VulkanLogicalDevice> pLogicalDevice;
                    const char* Name = "Image View";
            };
            VulkanImageView(const ImageViewDesc& desc);
            VkImageView GetView() const { return m_ImageView; }

        private:
            ImageViewWrapper m_ImageView;
            ImageViewDesc m_Desc;
    };

    class VulkanTexture
    {
            DELETE_COPY(VulkanTexture);

        public:
            struct CreateInfo
            {
                    VkExtent2D Extent;
                    VkFormat Format;
                    VkImageTiling Tiling = VK_IMAGE_TILING_OPTIMAL;
                    VkImageUsageFlags UsageFlags;
                    VkImageAspectFlagBits AspectFlags;
                    VkMemoryPropertyFlags MemoryPropertiesFlags;
                    class RenderDevice* pRenderDevice;
                    const char* Name    = "Default Texture";
                    float MaxAnisotropy = 1.0f;
            };

        public:
            VulkanTexture(VulkanTexture::CreateInfo& info);

            std::shared_ptr<VulkanImage> GetImage() const { return m_Image; }
            VkImageAspectFlagBits GetAspect() const { return m_Info.AspectFlags; }
            VkExtent2D GetExtent() const { return m_Info.Extent; }

        public:
            VkDescriptorImageInfo DescriptorInfo;

        private:
            std::shared_ptr<VulkanImage> m_Image;
            std::shared_ptr<VulkanImageView> m_ImageView;
            SamplerWrapper m_Sampler;
            VulkanTexture::CreateInfo& m_Info;
    };

}  // namespace ENGINE_NAMESPACE
