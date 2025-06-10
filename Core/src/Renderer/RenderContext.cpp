#include "RenderContext.h"
#include "Renderer.h"
#include "VulkanQueue.h"
#include "VulkanCommandBufferPool.h"
#include "TypeConversions.h"

#include "src/Core/Assert.h"
#include "src/Core/Ref.h"

#include <cstring>
#include <memory>

namespace fg {
RenderContext::RenderContext(ReferenceCounter* refCounter, Ref<Renderer> renderer,
                             const RenderContextInfo& info)
    : RefBase(refCounter), m_Renderer(renderer), m_Info(info) {
  auto device = m_Renderer->GetLogicalDevice();
  uint32_t queuIndex = m_Renderer->GetQueue()->GetFamilyIndex();
  m_CmdPool = std::make_unique<VulkanCommandBufferPool>(
      m_Renderer->GetLogicalDevice(), queuIndex,
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  PrepareCmdBuffer();
  m_BoundImages.reserve(8);
}

void RenderContext ::PrepareCmdBuffer() {
  if (m_Cmd.Get() == VK_NULL_HANDLE) {
    auto cmd = m_CmdPool->Get();
    m_Cmd.SetVkCommandBuffer(cmd, 0, 0);
  }
}

void RenderContext::BindDescriptorSets(VulkanDescriptorSet* descriptorSets[], uint32_t setCount) {
  FOO_ASSERT(m_BoundPipeline != nullptr);
  FOO_ASSERT(setCount > 0);
  FOO_ASSERT(descriptorSets != nullptr);
  VkDescriptorSet sets[16];
  memset(sets, 0, sizeof(sets));
  for (uint32_t i = 0; i < setCount; i++) {
    sets[i] = descriptorSets[i]->GetHandle();
  }
  m_Cmd.BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, m_BoundPipeline->Layout(), 0, setCount,
                           sets, 0, nullptr);
}

void RenderContext::BindVertexBuffers(const VertexBufferBindingAttr& attr) {
  FOO_ASSERT(m_BoundPipeline != nullptr);
  FOO_ASSERT(attr.BindingCount > 0);
  FOO_ASSERT(attr.ppBuffers != nullptr);
  FOO_ASSERT(attr.Offsets != nullptr);
  FOO_ASSERT(attr.ppBuffers != nullptr);
  VkBuffer vkbuffers[16];
  memset(vkbuffers, 0, sizeof(vkbuffers));
  for (uint32_t i = 0; i < attr.BindingCount; i++) {
    vkbuffers[i] = attr.ppBuffers[i]->GetVkBuffer();
  }
  m_Cmd.BindVertexBuffers(attr.FirstBinding, attr.BindingCount, vkbuffers, attr.Offsets);
}

void RenderContext::Draw(const DrawAttributes& attr) {
  FOO_ASSERT(m_BoundPipeline != nullptr);
  m_Cmd.Draw(attr.VertexCount, attr.InstanceCount, attr.FirstVertex, attr.FirstInstance);
}

void RenderContext::Flush() {
  auto vkCmdBuffer = m_Cmd.Get();
  if (m_Cmd.GetState().InsideRendering) {
    m_Cmd.EndRendering();
    m_Cmd.FlushBarriers();
    m_Cmd.EndCommandBuffer();
  }
  VkSubmitInfo submitInfo {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &vkCmdBuffer;
  submitInfo.waitSemaphoreCount = m_SignalSemaphores.size();
  submitInfo.pWaitSemaphores = m_SignalSemaphores.data();
  m_Renderer->ExecuteCommandBuffer(submitInfo, VK_NULL_HANDLE);

  m_SignalSemaphores.clear();

  DisposeCurrentCmdBuffer();
  m_BoundDepthStencil = nullptr;
  m_BoundPipeline = nullptr;
  m_BoundImages.clear();
}
void RenderContext::DisposeCurrentCmdBuffer() {
  FOO_ASSERT(!m_Cmd.GetState().InsideRendering, "Disposing cmd buffer while rendering");
  auto vkcmd = m_Cmd.Get();
  if (vkcmd != VK_NULL_HANDLE) {
    DisposeVkCmdBuffer(vkcmd);
    m_Cmd.Reset();
  }
}
void RenderContext::DisposeVkCmdBuffer(VkCommandBuffer cmd) {
  m_CmdPool->Recycle(std::move(cmd));
}

void RenderContext::FinishFrame() {
  m_FrameNumber++;
}

void RenderContext::BindPipeline(Ref<VulkanGraphicsPipeline> pipeline) {
  FOO_ASSERT(m_BoundImages.size() != 0);
  m_BoundPipeline = pipeline;
  if (m_BoundPipeline) {
    m_Cmd.BindGraphicsPipeline(m_BoundPipeline->GetHandle());
  }
}

void RenderContext::SetRenderTargets(const RenderTargetAttr& attr) {
  m_BoundImages.clear();
  PrepareCmdBuffer();
  VkImageSubresourceRange range {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  for (uint32_t i = 0; i < attr.RenderTargetCount; i++) {
    auto* image = attr.ppRenderTargets[i]->GetImage();
    m_BoundImages.push_back(image);
    m_Cmd.TransitionImageLayout(
        image->GetVkImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        range, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
  }
  const auto* image = attr.ppRenderTargets[0]->GetImage();

  VkRenderingInfoKHR beginInfo {VK_STRUCTURE_TYPE_RENDERING_INFO, 0};
  beginInfo.renderArea = {
      {0, 0}
  };
  beginInfo.renderArea.extent.width = image->Width();
  beginInfo.renderArea.extent.height = image->Height();
  beginInfo.layerCount = 1;

  VkRenderingAttachmentInfo colorInfos[8];
  memset(colorInfos, 0, sizeof(colorInfos));

  for (uint32_t i = 0; i < attr.RenderTargetCount; i++) {
    auto& colorInfo = colorInfos[i];
    colorInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorInfo.imageView = attr.ppRenderTargets[i]->GetHandle();
    colorInfo.clearValue = {
        {0.2f, 0.2f, 0.2f, 1.0f}
    };
  }
  beginInfo.colorAttachmentCount = attr.RenderTargetCount;
  beginInfo.pColorAttachments = colorInfos;
  VkRenderingAttachmentInfo depthInfo {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR};
  depthInfo.clearValue = {};
  depthInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depthInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  if (attr.DepthStencil) {
    depthInfo.imageView = attr.DepthStencil->GetHandle();
    m_BoundDepthStencil = attr.DepthStencil;
  }
  beginInfo.pDepthAttachment = attr.DepthStencil != nullptr ? &depthInfo : nullptr;
  m_Cmd.BeginRendering(beginInfo);
  VkRect2D scissor {
      {             0,               0},
      {image->Width(), image->Height()},
  };
  m_Cmd.SetScissor(scissor);
  VkViewport vp {0, 0, (float)image->Width(), (float)image->Height(), 0.0f, 1.0f};
  m_Cmd.CmdSetViewport(0, 1, vp);
}

void RenderContext::EndRendering() {
  m_Cmd.EndRendering();
  VkImageSubresourceRange range {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  for (auto& image : m_BoundImages) {
    m_Cmd.TransitionImageLayout(image->GetVkImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range,
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
  }
  m_Cmd.FlushBarriers();
}

void RenderContext::TransitionImageLayout(VulkanImage* image, VkImageLayout newLayout) {
  if (!image->IsInKnownState()) {
    FOO_CORE_ERROR("Can not transition image because image is in unknown state");
    return;
  }
  auto newState = VkImageLayoutToResouceState(newLayout);
  if (!image->CheckState(newState)) {
    TransitionImageState(*image, ResourceState::Unknown, newState);
  }
}

void RenderContext::TransitionImageState(VulkanImage& image, ResourceState oldState,
                                         ResourceState newState) {
  if (oldState == ResourceState::Unknown) {
    if (image.IsInKnownState()) {
      oldState = image.State();
    } else {
      FOO_CORE_ERROR("Failed to transition the state of the texture");
    }
  } else {
    if (image.IsInKnownState() && image.State() != oldState) {
      FOO_CORE_ERROR("State is not match");
    }
  }
  auto vkImg = image.GetVkImage();
  VkImageSubresourceRange range {};
  range.aspectMask = 0;
  range.baseArrayLayer = 0;
  range.layerCount = VK_REMAINING_ARRAY_LAYERS;
  range.baseMipLevel = 0;
  range.levelCount = VK_REMAINING_MIP_LEVELS;

  const auto& desc = image.GetDesc();
  if (desc.Format == ImageFormat::D32) {
    range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  } else {
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }
  const bool hasWriteAccess = ResourceStateHasWriteAccess(oldState);
  const auto oldLayout = ResourceStateToVkImageLayout(oldState);
  const auto newLayout = ResourceStateToVkImageLayout(newState);
  const auto oldStages = ResourceStateFlagsToVkPipelineStageFlags(oldState);
  const auto newStages = ResourceStateFlagsToVkPipelineStageFlags(newState);
  m_Cmd.TransitionImageLayout(vkImg, oldLayout, newLayout, range, oldStages, newStages);
  image.SetState(newState);
}

void RenderContext::AddSignalSemaphore(VkSemaphore sem) {
  m_SignalSemaphores.emplace_back(sem);
}

}  // namespace fg
