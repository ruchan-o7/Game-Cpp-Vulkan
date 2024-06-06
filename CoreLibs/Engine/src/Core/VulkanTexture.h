#pragma once
#include <memory>
#include "VulkanLogicalDevice.h"
#include "Utils/VulkanObjectWrapper.h"
namespace ENGINE_NAMESPACE
{
    namespace Texture
    {
        enum class Type
        {
            Texture2D,
            Texture3D,
        };
        enum class Format
        {
            Unknown,
            Png,
            Jpeg_Jpg,
        };
        struct TextureDescription
        {
                std::string Name = "Default Texture Name";
                uint32_t Width, Height;
                Type TextureType          = Type::Texture2D;
                Format TextureFormat      = Format::Unknown;
                uint32_t MipLevels        = 1;
                uint32_t SampleCount      = 1;
                VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        };
        struct TextureData
        {
                void* Data;
                size_t Size;
                std::shared_ptr<VulkanLogicalDevice> LogicalDevice;
        };

    };  // namespace Texture
    class VulkanTexture
    {
        public:
            VulkanTexture(const Texture::TextureDescription& desc,
                          const Texture::TextureData& data);
            VkImage GetVkImage() const { return m_Image; }
            VkSampler GetSampler() const { return m_Sampler; }

        private:
            ImageWrapper m_Image;
            ImageViewWrapper m_ImageView;
            DeviceMemoryWrapper m_Memory;
            SamplerWrapper m_Sampler;
            std::shared_ptr<VulkanLogicalDevice> m_pLogicalDevice;
            size_t m_Size;
            void* m_pTextureData;
    };
}  // namespace ENGINE_NAMESPACE
