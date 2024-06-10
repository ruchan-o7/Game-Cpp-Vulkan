#include "VulkanTexture.h"
#include "RenderDevice.h"
#include <cassert>
namespace ENGINE_NAMESPACE
{
    VulkanTexture::VulkanTexture(const TextureDescription& desc, const TextureData& data)
        : m_Image(nullptr),
          m_Memory(nullptr),
          m_Sampler(nullptr),
          m_ImageView(nullptr),
          m_pLogicalDevice(data.LogicalDevice),
          m_Size(data.Size),
          m_pTextureData(data.Data)
    {
        assert(0 && "NOT IMPLEMENTED YET");
    }

    ImageView::ImageView(const ImageViewDesc& desc) : m_Desc(desc)
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
        m_Desc.pLogicalDevice->CreateImageView(viewInfo, m_Desc.Name);
    }

    Image::Image(const ImageDesc& desc) : m_Desc(desc)
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
}  // namespace ENGINE_NAMESPACE
