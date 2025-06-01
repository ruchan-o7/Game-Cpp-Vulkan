#include "VulkanImage.h"
#include "Renderer.h"
#include "../Renderer/VulkanPhysicalDevice.h"

namespace fg {

VulkanImage::VulkanImage(Renderer* renderer, const ImageDescription& desc, const Buffer buffer)
    : m_Renderer(renderer), m_Desc(desc) {
  VkImageCreateInfo info {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, VK_NULL_HANDLE, 0};
  info.imageType = m_Desc.Type;
  info.format = m_Desc.Format;
  info.extent = {m_Desc.Width, m_Desc.Height, m_Desc.Depth};
  info.mipLevels = m_Desc.MipLevels;
  info.arrayLayers = 1;
  info.samples = VK_SAMPLE_COUNT_1_BIT;
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.usage = m_Desc.Usage;
  info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  info.queueFamilyIndexCount = 0;
  info.pQueueFamilyIndices = 0;
  info.initialLayout = m_Desc.InitialLayout;
  SetState(ResourceState::Undefined);
  auto lDev = m_Renderer->GetLogicalDevice();
  m_Image = lDev->CreateImage(info, m_Desc.Name);

  VkMemoryRequirements memReqs = lDev->GetImageMemReq(m_Image);
  VkMemoryAllocateInfo allocInfo {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

  allocInfo.memoryTypeIndex = renderer->GetPhysicalDevice().FindMemTypeIndex(
      memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  m_Memory = lDev->AllocateMemory(allocInfo);
  lDev->BindImageMemory(m_Image, m_Memory);
  if (buffer) {
    // Load texture data
  }

  if (m_Desc.Stage) {
    // NOTE: do stage buffering
  }
}

VkImageView VulkanImage::CreateView() {
  return 0;
}

}  // namespace fg
