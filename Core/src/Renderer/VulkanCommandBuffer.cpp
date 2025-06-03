#include "VulkanCommandBuffer.h"
#include "../Core/Assert.h"

namespace {

// see Diligent-Engine Vk Impl
static VkAccessFlags AccessMaskFromImageLayout(VkImageLayout Layout,
                                               bool IsDstMask  // false - source mask
                                                               // true  - destination mask
) {
  VkAccessFlags AccessMask = 0;
  switch (Layout) {
    // does not support device access. This layout must only be used as the initialLayout member
    // of VkImageCreateInfo or VkAttachmentDescription, or as the oldLayout in an image transition.
    // When transitioning out of this layout, the contents of the memory are not guaranteed to be
    // preserved (11.4)
    case VK_IMAGE_LAYOUT_UNDEFINED:
      if (IsDstMask) {
        FOO_ASSERT(
            false,
            "The new layout used in a transition must not be VK_IMAGE_LAYOUT_UNDEFINED. "
            "This layout must only be used as the initialLayout member of VkImageCreateInfo "
            "or VkAttachmentDescription, or as the oldLayout in an image transition. (11.4)");
      }
      break;

    // supports all types of device access
    case VK_IMAGE_LAYOUT_GENERAL:
      // VK_IMAGE_LAYOUT_GENERAL must be used for image load/store operations (13.1.1, 13.2.4)
      AccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
      break;

    // must only be used as a color or resolve attachment in a VkFramebuffer (11.4)
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      AccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;

    // must only be used as a depth/stencil attachment in a VkFramebuffer (11.4)
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      AccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;

    // must only be used as a read-only depth/stencil attachment in a VkFramebuffer and/or as a
    // read-only image in a shader (11.4)
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
      AccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
      break;

    // must only be used as a read-only image in a shader (which can be read as a sampled image,
    // combined image/sampler and/or input attachment) (11.4)
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      AccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
      break;

    //  must only be used as a source image of a transfer command (11.4)
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      AccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;

    // must only be used as a destination image of a transfer command (11.4)
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      AccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;

    // does not support device access. This layout must only be used as the initialLayout member
    // of VkImageCreateInfo or VkAttachmentDescription, or as the oldLayout in an image transition.
    // When transitioning out of this layout, the contents of the memory are preserved. (11.4)
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      if (!IsDstMask) {
        AccessMask = VK_ACCESS_HOST_WRITE_BIT;
      } else {
        FOO_ASSERT(
            false,
            "The new layout used in a transition must not be VK_IMAGE_LAYOUT_PREINITIALIZED. "
            "This layout must only be used as the initialLayout member of VkImageCreateInfo "
            "or VkAttachmentDescription, or as the oldLayout in an image transition. (11.4)");
      }
      break;

    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
      AccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
      break;

    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
      AccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
      break;

    // When transitioning the image to VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR or
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, there is no need to delay subsequent processing, or perform
    // any visibility operations (as vkQueuePresentKHR performs automatic visibility operations). To
    // achieve this, the dstAccessMask member of the VkImageMemoryBarrier should be set to 0, and
    // the dstStageMask parameter should be set to VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT.
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      AccessMask = 0;
      break;

    case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
      AccessMask = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
      break;

    case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
      AccessMask = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
      break;

    default:
      FOO_ASSERT(false, "Unexpected image layout");
      break;
  }

  return AccessMask;
}
}  // namespace

namespace fg {

VulkanCommandBuffer::VulkanCommandBuffer() noexcept {
  m_ImageBarriers.reserve(32);
}

void VulkanCommandBuffer::FlushBarriers() {
  if (m_PipelineBarrier.MemSrcStages == 0 && m_PipelineBarrier.MemDstStages == 0 &&
      m_ImageBarriers.empty()) {
    return;
  }

  if (m_State.InsideRendering) {
    EndRendering();
  }

  FOO_ASSERT(m_Cmd != VK_NULL_HANDLE);

  VkMemoryBarrier vkMemBarrier {};
  vkMemBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  vkMemBarrier.pNext = nullptr;

  const bool HasMemoryBarrier =
      m_PipelineBarrier.MemSrcStages != 0 && m_PipelineBarrier.MemDstStages != 0 &&
      m_PipelineBarrier.MemSrcAccess != 0 && m_PipelineBarrier.MemDstAccess != 0;

  const VkPipelineStageFlags SrcStages =
      (m_PipelineBarrier.ImageSrcStages | m_PipelineBarrier.MemSrcStages);
  const VkPipelineStageFlags DstStages =
      (m_PipelineBarrier.ImageDstStages | m_PipelineBarrier.MemDstStages);
  FOO_ASSERT(SrcStages != 0 && DstStages != 0);

  vkCmdPipelineBarrier(m_Cmd, SrcStages, DstStages, 0, HasMemoryBarrier ? 1 : 0,
                       HasMemoryBarrier ? &vkMemBarrier : nullptr, 0, nullptr,
                       static_cast<uint32_t>(m_ImageBarriers.size()),
                       m_ImageBarriers.empty() ? nullptr : m_ImageBarriers.data());

  m_ImageBarriers.clear();
  m_PipelineBarrier.ImageSrcStages = 0;
  m_PipelineBarrier.ImageDstStages = 0;
  m_PipelineBarrier.MemSrcStages = 0;
  m_PipelineBarrier.MemDstStages = 0;
  m_PipelineBarrier.MemSrcAccess = 0;
  m_PipelineBarrier.MemDstAccess = 0;
}

void VulkanCommandBuffer::TransitionImageLayout(VkImage image, VkImageLayout oldLayout,
                                                VkImageLayout newLayout,
                                                const VkImageSubresourceRange& range,
                                                VkPipelineStageFlags srcStages,
                                                VkPipelineStageFlags dstStages) {
  if (m_State.InsideRendering) {
    EndRendering();
  }

  if (oldLayout == newLayout) {
    m_PipelineBarrier.MemSrcStages |= srcStages;
    m_PipelineBarrier.MemDstStages |= dstStages;

    m_PipelineBarrier.MemSrcAccess |= AccessMaskFromImageLayout(oldLayout, false);
    m_PipelineBarrier.MemDstAccess |= AccessMaskFromImageLayout(newLayout, true);
    return;
  }

  m_PipelineBarrier.ImageSrcStages |= srcStages;
  m_PipelineBarrier.ImageDstStages |= dstStages;

  VkImageMemoryBarrier ImgBarrier {};
  ImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  ImgBarrier.pNext = nullptr;
  ImgBarrier.oldLayout = oldLayout;
  ImgBarrier.newLayout = newLayout;
  ImgBarrier.image = image;
  ImgBarrier.subresourceRange = range;
  ImgBarrier.srcAccessMask = AccessMaskFromImageLayout(oldLayout, false);
  ImgBarrier.dstAccessMask = AccessMaskFromImageLayout(newLayout, true);

  // source queue family for a queue family ownership transfer.
  ImgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  // destination queue family for a queue family ownership transfer.
  ImgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  m_ImageBarriers.emplace_back(ImgBarrier);
}

}  // namespace fg
