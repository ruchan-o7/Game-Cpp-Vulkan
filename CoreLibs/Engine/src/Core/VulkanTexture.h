#pragma once
#include <memory>
#include "VulkanLogicalDevice.h"
#include "Types.h"
#include "Utils/VulkanObjectWrapper.h"
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
}  // namespace ENGINE_NAMESPACE
