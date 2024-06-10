#pragma once
#include <memory>
#include "VulkanLogicalDevice.h"
#include "Types.h"
#include "Utils/VulkanObjectWrapper.h"
#include "vulkan/vulkan_core.h"
namespace ENGINE_NAMESPACE
{
    class VulkanTexture
    {
        public:
            VulkanTexture(const TextureDescription& desc, const TextureData& data);

            void Create();
            VkImage GetVkImage() const { return m_Image; }
            VkSampler GetSampler() const { return m_Sampler; }

            void TransitionImageLayout();

        private:
            ImageWrapper m_Image;
            ImageViewWrapper m_ImageView;
            DeviceMemoryWrapper m_Memory;
            SamplerWrapper m_Sampler;

            std::shared_ptr<VulkanLogicalDevice> m_pLogicalDevice;

            size_t m_Size;
            void* m_pTextureData;
    };
    class Image
    {
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
            Image(const ImageDesc& desc);

        private:
            ImageDesc m_Desc;
            ImageWrapper m_Image;
            DeviceMemoryWrapper m_ImageMemory;
    };
    class ImageView
    {
        public:
            struct ImageViewDesc
            {
                    VkImage Image;
                    VkFormat Format;
                    VkImageAspectFlagBits Aspects;
                    std::shared_ptr<VulkanLogicalDevice> pLogicalDevice;
                    const char* Name = "Image View";
            };
            ImageView(const ImageViewDesc& desc);

        private:
            ImageViewWrapper m_ImageView;
            ImageViewDesc m_Desc;
    };

}  // namespace ENGINE_NAMESPACE
