#include "VulkanSwapchain.h"
#include "GLFW/glfw3.h"

#include "RenderContext.h"
#include "VulkanInstance.h"
#include "VulkanLogicalDevice.h"
#include "VulkanPhysicalDevice.h"
#include "Renderer.h"
#include "VulkanImage.h"
#include "VulkanQueue.h"

#include "../Core/Log.h"
#include <stdexcept>

namespace fg {

VulkanSwapchain::VulkanSwapchain(ReferenceCounter* counter, GLFWwindow* window,
                                 Ref<Renderer> renderer, RenderContext* context)
    : RefBase(counter), m_Window(window), m_Renderer(renderer), m_Ctx(context) {
  CreateSurface();
  CreateSwapchain();
  AcquireNextImage(context);
}

VulkanSwapchain::~VulkanSwapchain() {
  m_BackbufferRtvs.clear();
  m_ImageAvailable.Release();
  m_RenderFinished.Release();
  m_InFlight.Release();
  auto device = m_Renderer->GetLogicalDevice();
  if (m_Swapchain) {
    vkDestroySwapchainKHR(device->GetHandle(), m_Swapchain, device->GetAllocator());
  }
  if (m_Surface) {
    vkDestroySurfaceKHR(m_Renderer->GetVkInstance2().GetHandle(), m_Surface,
                        device->GetAllocator());
  }
}

void VulkanSwapchain::CreateSurface() {
  auto& vkInstance = m_Renderer->GetVkInstance2();
  auto res = glfwCreateWindowSurface(vkInstance.GetHandle(), m_Window, vkInstance.GetAllocator(),
                                     &m_Surface);
  if (res != VK_SUCCESS) {
    FOO_CORE_ERROR("Can not create window surface");
    throw std::runtime_error("Can not create window surface");
  }
}

void VulkanSwapchain::CreateSwapchain() {
  auto& physicalDevice = m_Renderer->GetPhysicalDevice();
  auto& vkInstance = m_Renderer->GetVkInstance2();
  auto device = m_Renderer->GetLogicalDevice();
  uint32_t surfaceCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.GetHandle(), m_Surface, &surfaceCount,
                                       nullptr);
  VkSurfaceFormatKHR formats[16];
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.GetHandle(), m_Surface, &surfaceCount,
                                       formats);
  for (uint32_t i = 0; i < surfaceCount; i++) {
    // TODO: Improve
    const auto& format = formats[i];
    m_SurfaceFormat = format;
  }
  m_SurfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
  m_SurfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

  VkSurfaceCapabilitiesKHR caps {};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.GetHandle(), m_Surface, &caps);

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
  auto res =
      vkCreateSwapchainKHR(device->GetHandle(), &info, vkInstance.GetAllocator(), &m_Swapchain);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Can not create swapchain");
  }
  uint32_t viewCount = 0;
  vkGetSwapchainImagesKHR(device->GetHandle(), m_Swapchain, &viewCount, nullptr);
  std::vector<VkImage> images;
  images.resize(viewCount);
  vkGetSwapchainImagesKHR(device->GetHandle(), m_Swapchain, &viewCount, images.data());

  m_BackbufferRtvs.resize(viewCount);
  for (uint32_t i = 0; i < viewCount; i++) {
    ImageDescription backBufferDesc;
    backBufferDesc.Type = ImageType::Type2D;
    backBufferDesc.Width = m_Extent.width;
    backBufferDesc.Height = m_Extent.height;
    backBufferDesc.Format = ImageFormat::RGBA8Unorm;
    backBufferDesc.Usage = ImageUsage::ColorAttachment;
    backBufferDesc.MipLevels = 1;
    Ref<VulkanImage> backBufferImage =
        m_Renderer->CreateImage(backBufferDesc, ResourceState::Undefined, images[i]);

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
  m_InFlight = device->CreateFence(fenceInfo);
  m_RenderFinished = device->CreateVulkanSemaphore(semInfo);
  m_ImageAvailable = device->CreateVulkanSemaphore(semInfo);
}

void VulkanSwapchain::RecreateSwapchain() {
  auto& physicalDevice = m_Renderer->GetPhysicalDevice();
  auto device = m_Renderer->GetLogicalDevice();
  DestroySwapchainRes(false);
  VkSurfaceCapabilitiesKHR caps {};
  auto err =
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.GetHandle(), m_Surface, &caps);
  if (err == VK_ERROR_SURFACE_LOST_KHR) {
    if (m_Swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device->GetHandle(), m_Swapchain, device->GetAllocator());
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
  auto device = m_Renderer->GetLogicalDevice();
  device->WaitIdle();
  WaitForImageAcquiredFences();
  m_BackbufferRtvs.clear();
  m_ImageAvailable.Release();
  m_RenderFinished.Release();
  m_InFlight.Release();
  m_FrameIndex = 0;
  if (destroySwapchain) {
    vkDestroySwapchainKHR(device->GetHandle(), m_Swapchain, device->GetAllocator());
    m_Swapchain = VK_NULL_HANDLE;
  }
}

void VulkanSwapchain::WaitForImageAcquiredFences() {
  auto device = m_Renderer->GetLogicalDevice();
  VkFence fence = m_InFlight;
  if (device->GetFenceStatus(fence) == VK_NOT_READY) {
    device->WaitFence(fence);
  }
}

VkResult VulkanSwapchain::AcquireNextImage(RenderContext* context) {
  auto device = m_Renderer->GetLogicalDevice();
  VkFence fence = m_InFlight;
  const auto fenceStatus = device->GetFenceStatus(fence);
  if (fenceStatus == VK_NOT_READY) {
    device->WaitFence(fence);
  }
  device->ResetFence(fence);
  auto res = vkAcquireNextImageKHR(device->GetHandle(), m_Swapchain, UINT64_MAX, m_ImageAvailable,
                                   fence, &m_FrameIndex);
  context->AddWaitSemaphore(m_ImageAvailable, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                                  VK_PIPELINE_STAGE_TRANSFER_BIT);
  return res;
}

void VulkanSwapchain::Present() {
  auto context = m_Ctx.Lock();
  if (!context) {
    FOO_CORE_ERROR("Context has been released");
    return;
  }

  auto* backBuffer = GetCurrentImageView()->GetImage();
  context->TransitionImageLayout(backBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  context->AddSignalSemaphore(m_RenderFinished);
  context->Flush();

  VkSemaphore waitSemaphores[] = {m_RenderFinished};

  VkPresentInfoKHR presentInfo {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = waitSemaphores;

  presentInfo.pSwapchains = &m_Swapchain;
  presentInfo.swapchainCount = 1;
  presentInfo.pImageIndices = &m_FrameIndex;
  VkResult result = VK_SUCCESS;
  presentInfo.pResults = &result;
  auto res = m_Renderer->Present(presentInfo);
  FOO_ASSERT(res == result);
  if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapchain();
    m_FrameIndex = m_ImageCount - 1;
  } else {
    if (result != VK_SUCCESS) {
      FOO_CORE_ERROR("Presentation failed");
    }
  }

  result = AcquireNextImage(context.get());
  if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapchain();
    m_FrameIndex = m_ImageCount - 1;
  } else {
    if (result != VK_SUCCESS) {
      FOO_CORE_ERROR("Presentation failed");
    }
  }
  context->FinishFrame();
}

}  // namespace fg
