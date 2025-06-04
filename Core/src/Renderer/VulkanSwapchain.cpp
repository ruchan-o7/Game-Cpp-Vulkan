#include "VulkanSwapchain.h"
#include "GLFW/glfw3.h"

#include "VulkanInstance.h"
#include "VulkanLogicalDevice.h"
#include "VulkanPhysicalDevice.h"
#include "Renderer.h"
#include "VulkanImage.h"

#include "../Core/Log.h"
#include <stdexcept>

namespace fg {

// clang-format off
VulkanSwapchain::VulkanSwapchain(GLFWwindow* window, 
                    std::weak_ptr<Renderer> renderer,
                    std::shared_ptr<VulkanInstance> instance,
                    std::shared_ptr<VulkanLogicalDevice> logicalDevice,
                    const VulkanPhysicalDevice& pDev)
    : m_Window(window),
      m_Renderer(renderer),
      m_Device(logicalDevice), 
      m_PhysicalDevice(pDev), 
      m_VkInstance(std::move(instance)) 
{
  // clang-format on
  CreateSurface();
  CreateSwapchain();
  AcquireNextImage();
}

VulkanSwapchain::~VulkanSwapchain() {
  m_BackbufferRtvs.clear();
  m_ImageAvailable.Release();
  m_RenderFinished.Release();
  m_InFlight.Release();
  if (m_Swapchain) {
    vkDestroySwapchainKHR(m_Device->GetHandle(), m_Swapchain, m_Device->GetAllocator());
  }
  if (m_Surface) {
    if (auto renderer = m_Renderer.lock()) {
      vkDestroySurfaceKHR(renderer->GetVkInstance2().GetHandle(), m_Surface,
                          m_Device->GetAllocator());
    }
  }
}

void VulkanSwapchain::CreateSurface() {
  auto res = glfwCreateWindowSurface(m_VkInstance->GetHandle(), m_Window,
                                     m_VkInstance->GetAllocator(), &m_Surface);
  if (res != VK_SUCCESS) {
    FOO_CORE_ERROR("Can not create window surface");
    throw std::runtime_error("Can not create window surface");
  }
}
void VulkanSwapchain::CreateSwapchain() {
  uint32_t surfaceCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice.GetHandle(), m_Surface, &surfaceCount,
                                       nullptr);
  VkSurfaceFormatKHR formats[16];
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice.GetHandle(), m_Surface, &surfaceCount,
                                       formats);
  for (uint32_t i = 0; i < surfaceCount; i++) {
    // TODO: Improve
    const auto& format = formats[i];
    m_SurfaceFormat = format;
  }
  m_SurfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
  m_SurfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

  VkSurfaceCapabilitiesKHR caps {};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice.GetHandle(), m_Surface, &caps);

  m_Extent.width = caps.currentExtent.width;
  m_Extent.height = caps.currentExtent.height;

  m_ImageCount = caps.minImageCount + 1;
  VkSwapchainKHR oldSwapchain = m_Swapchain;

  VkSwapchainCreateInfoKHR info {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  info.surface = m_Surface;
  info.minImageCount = m_ImageCount;
  info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  info.imageColorSpace = m_SurfaceFormat.colorSpace;
  info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  info.imageFormat = m_SurfaceFormat.format;
  info.imageExtent = m_Extent;
  info.imageArrayLayers = 1;
  info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  info.clipped = VK_TRUE;
  info.oldSwapchain = oldSwapchain;
  info.queueFamilyIndexCount = 1;
  uint32_t famIndicies[] = {0};
  info.pQueueFamilyIndices = famIndicies;
  info.preTransform = caps.currentTransform;
  info.presentMode = m_PresentMode;
  auto res = vkCreateSwapchainKHR(m_Device->GetHandle(), &info, m_VkInstance->GetAllocator(),
                                  &m_Swapchain);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Can not create swapchain");
  }
  uint32_t viewCount = 0;
  vkGetSwapchainImagesKHR(m_Device->GetHandle(), m_Swapchain, &viewCount, nullptr);
  m_Images.resize(viewCount);
  vkGetSwapchainImagesKHR(m_Device->GetHandle(), m_Swapchain, &viewCount, m_Images.data());

  m_BackbufferRtvs.resize(viewCount);
  auto renderer = m_Renderer.lock();
  for (uint32_t i = 0; i < viewCount; i++) {
    ImageDescription backBufferDesc;
    backBufferDesc.Type = ImageType::Type2D;
    backBufferDesc.Width = m_Extent.width;
    backBufferDesc.Height = m_Extent.height;
    backBufferDesc.Format = ImageFormat::RGBA8Unorm;
    backBufferDesc.Usage = ImageUsage::ColorAttachment;
    backBufferDesc.MipLevels = 1;
    Ref<VulkanImage> backBufferImage =
        renderer->CreateImage(backBufferDesc, ResourceState::Undefined, m_Images[i]);

    ImageViewDesc rtViewDesc;
    rtViewDesc.ViewType = ImageViewType::RenderTarget;
    rtViewDesc.Type = ImageType::Type2D;
    rtViewDesc.Format = backBufferDesc.Format;
    m_BackbufferRtvs[i] = backBufferImage->CreateView(rtViewDesc);
  }
  m_InFlight = {};
  m_RenderFinished = {};
  m_ImageAvailable = {};

  VkSemaphoreCreateInfo semInfo {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  VkFenceCreateInfo fenceInfo {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  m_InFlight = m_Device->CreateFence(fenceInfo);
  m_RenderFinished = m_Device->CreateVulkanSemaphore(semInfo);
  m_ImageAvailable = m_Device->CreateVulkanSemaphore(semInfo);
}

void VulkanSwapchain::RecreateSwapchain() {
  DestroySwapchainRes(false);
  VkSurfaceCapabilitiesKHR caps {};
  auto err =
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice.GetHandle(), m_Surface, &caps);
  if (err == VK_ERROR_SURFACE_LOST_KHR) {
    if (m_Swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(m_Device->GetHandle(), m_Swapchain, m_Device->GetAllocator());
      m_Swapchain = VK_NULL_HANDLE;
    }
    CreateSurface();
  }
  CreateSwapchain();
}
void VulkanSwapchain::DestroySwapchainRes(bool destroySwapchain) {
  if (m_Swapchain == VK_NULL_HANDLE) {
    return;
  }
  m_Device->WaitIdle();
  WaitForImageAcquiredFences();
  m_BackbufferRtvs.clear();
  m_Images.clear();
  m_ImageAvailable.Release();
  m_RenderFinished.Release();
  m_InFlight.Release();
  m_FrameIndex = 0;
  if (destroySwapchain) {
    vkDestroySwapchainKHR(m_Device->GetHandle(), m_Swapchain, m_Device->GetAllocator());
    m_Swapchain = VK_NULL_HANDLE;
  }
}

void VulkanSwapchain::WaitForImageAcquiredFences() {
  VkFence fence = m_InFlight;
  if (m_Device->GetFenceStatus(fence) == VK_NOT_READY) {
    m_Device->WaitFence(fence);
  }
}

VkResult VulkanSwapchain::AcquireNextImage() {
  m_Device->WaitFence(m_InFlight);
  VkFence fence = m_InFlight;
  m_Device->ResetFence(fence);
  return vkAcquireNextImageKHR(m_Device->GetHandle(), m_Swapchain, UINT64_MAX, m_ImageAvailable,
                               VK_NULL_HANDLE, &m_FrameIndex);
}

void VulkanSwapchain::Present() {
  auto renderer = m_Renderer.lock();
  auto* backBuffer = GetCurrentImageView()->GetImage();
  renderer->TransitionImageLayout(backBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  renderer->Flush();

  VkSubmitInfo submit {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  VkSemaphore waits[] = {m_ImageAvailable};

  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit.waitSemaphoreCount = 1;
  submit.pWaitSemaphores = waits;
  submit.pWaitDstStageMask = waitStages;
  submit.commandBufferCount = 1;

  VkSemaphore signalSems[] = {m_RenderFinished};
  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores = signalSems;
  if (!renderer) {
    FOO_CORE_ERROR("Renderer disposed before swapchain");
    return;
  }
  auto res = renderer->Flush([&](VkQueue queue, VkCommandBuffer cmd) -> VkResult {
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    return vkQueueSubmit(queue, 1, &submit, m_InFlight);
  });

  VkPresentInfoKHR presentInfo {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSems;
  presentInfo.pSwapchains = &m_Swapchain;
  presentInfo.swapchainCount = 1;
  presentInfo.pImageIndices = &m_FrameIndex;
  VkResult result = VK_SUCCESS;
  presentInfo.pResults = &result;
  res = renderer->Present(presentInfo);
  if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapchain();
    m_FrameIndex = m_ImageCount - 1;
  } else {
    if (res != VK_SUCCESS) {
      FOO_CORE_ERROR("Presentation failed");
    }
  }

  res = AcquireNextImage();
  if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapchain();
    m_FrameIndex = m_ImageCount - 1;
  } else {
    if (res != VK_SUCCESS) {
      FOO_CORE_ERROR("Presentation failed");
    }
  }
}

}  // namespace fg
