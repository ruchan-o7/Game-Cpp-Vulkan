#include "VulkanImage.h"
#include "Renderer.h"
#include "../Core/Assert.h"
#include "vulkan/vulkan_core.h"

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
    case ImageFormat::RGBA8Unorm:
      return VK_FORMAT_R8G8B8A8_UNORM;
    case ImageFormat::RGB8:
      return VK_FORMAT_R8G8B8_SRGB;
    case ImageFormat::RGB8Unorm:
      return VK_FORMAT_R8G8B8_UNORM;
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
uint32_t ComponentSize(ImageFormat format) {
  switch (format) {
    case fg::ImageFormat::None:
      return 0;
    case fg::ImageFormat::RGBA8:
    case fg::ImageFormat::RGB8:
    case fg::ImageFormat::R8:
    case fg::ImageFormat::R8Unsigned:
      return sizeof(uint32_t);
    case fg::ImageFormat::D32:
    case fg::ImageFormat::RGBA16F:
      return sizeof(float);
  }
}

VulkanImage::VulkanImage(Renderer* renderer, const ImageDescription& desc,
                         ResourceState initialState, VkImage imageHandle)
    : m_Renderer(renderer), m_Desc(desc), m_State(initialState), m_VmaImage(imageHandle, nullptr) {
}

VulkanImage::VulkanImage(Renderer* renderer, const ImageDescription& desc, const Buffer buffer)
    : m_Renderer(renderer), m_Desc(desc) {
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
      BufferDescription stageDesc;
      stageDesc.Usage = BufferUsage::Staging;
      stageDesc.Name = "Staging buffer";
      stageDesc.Size = m_Desc.Width * m_Desc.Height * GetComponentCount(m_Desc.Format) *
                       ComponentSize(m_Desc.Format);
      auto stage = renderer->CreateBuffer(stageDesc, buffer);
      {
        CommandPoolWrapper cmdPool;
        VulkanCommandBuffer cmd;

        renderer->AllocateTransientCmdPool(cmdPool, cmd);
        VkImageSubresourceRange range {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        cmd.TransitionImageLayout(
            m_VmaImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
        VkBufferImageCopy region {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {Width(), Height(), Depth()};
        cmd.CopyBufferToImage(stage->GetVkBuffer(), m_VmaImage,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        cmd.TransitionImageLayout(m_VmaImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        cmd.FlushBarriers();
        renderer->ExecuteAndDisposeTransientCmdBuff(cmd.Get(), std::move(cmdPool));
      }
    }
  }
  CreateDefaultViews();
}
VkImageViewType ToVkView(ImageType type) {
  switch (type) {
    case ImageType::None:
      return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    case ImageType::Type1D:
      return VK_IMAGE_VIEW_TYPE_1D;
    case ImageType::Type2D:
      return VK_IMAGE_VIEW_TYPE_2D;
    case ImageType::Type3D:
      return VK_IMAGE_VIEW_TYPE_3D;
  }
}
bool IsDepthImage(ImageFormat format) {
  return format == ImageFormat::D32;
}
void VulkanImage::CreateDefaultViews() {
  VkImageViewCreateInfo info {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  info.image = m_VmaImage;
  info.viewType = ToVkView(m_Desc.Type);
  info.format = ToVk(m_Desc.Format);
  info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                     VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
  if (IsDepthImage(m_Desc.Format)) {
    info.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
  } else {
    info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  }
  auto view = m_Renderer->GetLogicalDevice()->CreateImageView(info);
  ImageViewDesc desc;
  // desc.ViewType = m_Desc.Type;
  desc.Format = m_Desc.Format;
  desc.Type = m_Desc.Type;
  m_DefaultView = MakeRef<VulkanImageView>(m_Renderer, desc, std::move(view), this);
}

Ref<VulkanImageView> VulkanImage::CreateView(const ImageViewDesc& desc) {
  VkImageViewCreateInfo info {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  info.image = m_VmaImage;
  info.viewType = ToVkView(desc.Type);
  info.format = ToVk(desc.Format);
  info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                     VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
  if (IsDepthImage(desc.Format)) {
    info.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
  } else {
    info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  }
  auto view = m_Renderer->GetLogicalDevice()->CreateImageView(info);
  return MakeRef<VulkanImageView>(m_Renderer, desc, std::move(view), Ref<VulkanImage>(this));
}

VulkanImageView::VulkanImageView(Renderer* renderer, const ImageViewDesc& desc,
                                 ImageViewWrapper&& view, VulkanImage* pImage)
    : m_Renderer(renderer), m_Desc(desc), m_View(std::move(view)), m_BaseImage(pImage) {
}

VulkanImageView::VulkanImageView(Renderer* renderer, const ImageViewDesc& desc,
                                 ImageViewWrapper&& view, Ref<VulkanImage> pImage)
    : m_Renderer(renderer), m_Desc(desc), m_View(std::move(view)), m_sBaseImage(pImage) {
}

VulkanImageView::~VulkanImageView() {
  // TODO: Safe release
  if (m_sBaseImage) {
    m_sBaseImage->Release();
  }
}
}  // namespace fg
