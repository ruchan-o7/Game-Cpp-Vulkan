#include "VulkanTexture.h"
#include "RenderDevice.h"
#include "VulkanBuffer.h"
namespace ENGINE_NAMESPACE
{

    VulkanImageView::VulkanImageView(const ImageViewDesc& desc) : m_Desc(desc)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = m_Desc.Image;
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                          = m_Desc.Format;
        viewInfo.subresourceRange.aspectMask     = m_Desc.Aspects;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;
        m_ImageView =  m_Desc.pLogicalDevice->CreateImageView(viewInfo, m_Desc.Name);
    }

    VulkanImage::VulkanImage(const ImageDesc& desc) : m_Desc(desc)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = m_Desc.Extent.width;
        imageInfo.extent.height = m_Desc.Extent.height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = m_Desc.Format;
        imageInfo.tiling        = m_Desc.Tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage         = m_Desc.Usage;
        imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

        m_Image = m_Desc.pRenderDevice->GetLogicalDevice()->CreateImage(imageInfo);

        auto memRequirements =
            m_Desc.pRenderDevice->GetLogicalDevice()->GetImageMemoryRequirements(m_Image);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = m_Desc.pRenderDevice->GetPhysicalDevice()->FindMemoryType(
            memRequirements.memoryTypeBits, desc.MemoryPropertiesFlags);

        m_ImageMemory = m_Desc.pRenderDevice->GetLogicalDevice()->AllocateDeviceMemory(
            allocInfo, "ImageMemory");

        m_Desc.pRenderDevice->GetLogicalDevice()->BindImageMemory(m_Image, m_ImageMemory, 0);
    }
    VulkanTexture::VulkanTexture(VulkanTexture::CreateInfo& info)
        : m_Info(info)  //, m_Image(nullptr), m_ImageView(nullptr)
    {
        VulkanImage::ImageDesc imageDesc{};
        imageDesc.Extent                = m_Info.Extent;
        imageDesc.Format                = m_Info.Format;
        imageDesc.Tiling                = m_Info.Tiling;
        imageDesc.Usage                 = m_Info.UsageFlags;
        imageDesc.MemoryPropertiesFlags = m_Info.MemoryPropertiesFlags;
        imageDesc.pRenderDevice         = m_Info.pRenderDevice;
        auto pI                         = new VulkanImage(imageDesc);
        m_Image                         = std::shared_ptr<VulkanImage>(pI);

        VulkanImageView::ImageViewDesc imageViewDesc{};
        imageViewDesc.Image          = m_Image->GetImageHandle();
        imageViewDesc.Format         = m_Info.Format;
        imageViewDesc.Aspects        = m_Info.AspectFlags;
        imageViewDesc.Name           = m_Info.Name;
        imageViewDesc.pLogicalDevice = m_Info.pRenderDevice->GetLogicalDevice();
        auto pIv                     = new VulkanImageView(imageViewDesc);
        m_ImageView                  = std::shared_ptr<VulkanImageView>(pIv);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = VK_FILTER_LINEAR;
        samplerInfo.minFilter               = VK_FILTER_LINEAR;
        samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable        = VK_TRUE;
        samplerInfo.maxAnisotropy           = m_Info.MaxAnisotropy;
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        m_Sampler = m_Info.pRenderDevice->CreateSampler(samplerInfo);

        DescriptorInfo.sampler     = m_Sampler;
        DescriptorInfo.imageView   = m_ImageView->GetView();
        DescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
}  // namespace ENGINE_NAMESPACE
