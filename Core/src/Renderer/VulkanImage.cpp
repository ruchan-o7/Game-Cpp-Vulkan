#include "VulkanImage.h"
#include "Renderer.h"
#include "../Core/Assert.h"

namespace fg {

VkImageUsageFlags ToVk(ImageUsage usage) {
  switch (usage) {
    case ImageUsage::None:
      FOO_CORE_ERROR("Image usage did not specified");
      return 0;
    case ImageUsage::Depth:
      return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    case ImageUsage::ColorAttachment:
      return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    case ImageUsage::Staging:
      return VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    case ImageUsage::ShaderResource:
      return VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
      break;
  }
}

uint32_t GetComponentCount(ImageFormat format) {
  switch (format) {
    case ImageFormat::None:
      return 0;
    case ImageFormat::RGBA16F:
    case ImageFormat::RGBA8:
      return 4;
    case ImageFormat::RGB8:
      return 3;
    case ImageFormat::R8:
    case ImageFormat::R8Unsigned:
    case ImageFormat::D32:
      return 1;
      break;
  }
  return 0;
}

VkFormat ToVk(ImageFormat format) {
  switch (format) {
    case ImageFormat::None:
      return VK_FORMAT_UNDEFINED;
    case ImageFormat::RGBA8:
      return VK_FORMAT_R8G8B8A8_SRGB;
    case ImageFormat::RGB8:
      return VK_FORMAT_R8G8B8_SRGB;
    case ImageFormat::RGBA16F:
      return VK_FORMAT_R16G16B16A16_SFLOAT;
    case ImageFormat::R8:
      return VK_FORMAT_R32_SINT;
    case ImageFormat::R8Unsigned:
      return VK_FORMAT_R32_UINT;
    case ImageFormat::D32:
      return VK_FORMAT_D32_SFLOAT;
      break;
  }
  return VK_FORMAT_UNDEFINED;
}
VkImageType ToVk(ImageType dim) {
  switch (dim) {
    case ImageType::Type1D:
      return VK_IMAGE_TYPE_1D;
    case ImageType::None:
    case ImageType::Type2D:
      return VK_IMAGE_TYPE_2D;
    case ImageType::Type3D:
      return VK_IMAGE_TYPE_3D;
  }
}

VulkanImage::VulkanImage(Renderer* renderer, const ImageDescription& desc, const Buffer buffer)
    : m_Desc(desc) {
  FOO_ASSERT(m_Desc.Type != ImageType::None);
  FOO_ASSERT(m_Desc.Format != ImageFormat::None);
  FOO_ASSERT(m_Desc.Usage != ImageUsage::None);
  FOO_ASSERT(m_Desc.Width != 0);
  FOO_ASSERT(m_Desc.Height != 0);
  if (m_Desc.Type == ImageType::Type2D) {
    FOO_ASSERT(m_Desc.Depth == 1);
  }
  if (m_Desc.Type == ImageType::Type3D) {
    FOO_ASSERT(m_Desc.Depth != 1);
  }
  VkImageCreateInfo info {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  info.imageType = ToVk(m_Desc.Type);
  info.extent.width = m_Desc.Width;
  info.extent.height = m_Desc.Height;
  info.extent.depth = m_Desc.Depth;
  info.mipLevels = m_Desc.MipLevels;
  info.arrayLayers = 1;
  info.format = ToVk(m_Desc.Format);
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  info.usage = ToVk(m_Desc.Usage);
  info.samples = VK_SAMPLE_COUNT_1_BIT;
  info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  SetState(ResourceState::Undefined);

  VmaAllocationCreateInfo allocInfo {};
  allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;  // TODO: Improve this
  allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

  // Division exception by 0. BRUHHHH!
  m_VmaImage = renderer->GetLogicalDevice()->CreateVMAImage(info, allocInfo);

  if (buffer) {
    if (m_Desc.Usage == ImageUsage::Staging) {
      // SetData(buffer);
    } else {
      BufferDescription stageDesc {};
      stageDesc.Usage = BufferUsage::Staging;
      stageDesc.Name = "Staging buffer";
      stageDesc.Size = m_Desc.Width * m_Desc.Height * GetComponentCount(m_Desc.Format);
      auto stage = renderer->CreateBuffer(stageDesc, buffer);
      stage->SetData(buffer);
      CopyBufferToImageAttr attr {};
      attr.Src = stage.get();
      attr.Dst = this;
      renderer->CopyBufferToImage(attr);
    }
  }
}

VkImageView VulkanImage::CreateView() {
  return 0;
}

}  // namespace fg
