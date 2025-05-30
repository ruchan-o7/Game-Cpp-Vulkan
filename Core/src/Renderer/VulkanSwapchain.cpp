#include "VulkanSwapchain.h"
#include "GLFW/glfw3.h"

#include "VulkanInstance.h"
#include "../Core/Log.h"
#include "VulkanLogicalDevice.h"
#include "VulkanPhysicalDevice.h"
#include "Renderer.h"
#include <stdexcept>

namespace fg {

// clang-format off
VulkanSwapchain::VulkanSwapchain(GLFWwindow* window, 
                     Renderer* renderer,
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
    const auto& format = formats[i];
    FOO_CORE_INFO("Surfece format: {}", (int)format.format);
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
  VkImage images[16];
  vkGetSwapchainImagesKHR(m_Device->GetHandle(), m_Swapchain, &viewCount, images);

  m_Views.resize(viewCount);

  VkImageViewCreateInfo viewInfo {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

  viewInfo.format = m_SurfaceFormat.format;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.components = {
      VK_COMPONENT_SWIZZLE_IDENTITY,
      VK_COMPONENT_SWIZZLE_IDENTITY,
      VK_COMPONENT_SWIZZLE_IDENTITY,
      VK_COMPONENT_SWIZZLE_IDENTITY,
  };
  viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  VkDevice dev = m_Device->GetHandle();
  for (int i = 0; i < m_Views.size(); i++) {
    viewInfo.image = images[i];
    vkCreateImageView(dev, &viewInfo, m_VkInstance->GetAllocator(), &m_Views[i]);
  }

  // for (auto& f : m_Fences) {
  //   vkDestroyFence(m_Device->GetHandle(), f, m_VkInstance->GetAllocator());
  // }
  // for (auto& s : m_Semaphores) {
  //   vkDestroySemaphore(m_Device->GetHandle(), s, m_VkInstance->GetAllocator());
  // }

  // m_Semaphores.resize(viewCount);
  // m_Fences.resize(viewCount);
  m_InFlight = {};
  m_RenderFinished = {};
  m_ImageAvailable = {};

  VkSemaphoreCreateInfo semInfo {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  VkFenceCreateInfo fenceInfo {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  m_InFlight = m_Device->CreateFence(fenceInfo);
  m_RenderFinished = m_Device->CreateVulkanSemaphore(semInfo);
  m_ImageAvailable = m_Device->CreateVulkanSemaphore(semInfo);
  // for (uint32_t i = 0; i < viewCount; i++) {
  //   vkCreateSemaphore(m_Device->GetHandle(), &semInfo, m_VkInstance->GetAllocator(),
  //                     &m_Semaphores[i]);
  //   vkCreateFence(m_Device->GetHandle(), &fenceInfo, m_VkInstance->GetAllocator(), &m_Fences[i]);
  // }
}

VkResult VulkanSwapchain::AcquireNextImage() {
  m_Device->WaitFence(m_InFlight);
  VkFence fence = m_InFlight;
  m_Device->ResetFence(fence);
  return vkAcquireNextImageKHR(m_Device->GetHandle(), m_Swapchain, UINT64_MAX, m_ImageAvailable,
                               VK_NULL_HANDLE, &m_FrameIndex);
}

void VulkanSwapchain::Present() {
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

  auto res = m_Renderer->Flush([&](VkQueue queue, VkCommandBuffer cmd) -> VkResult {
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
  m_Renderer->Present(presentInfo);

  res = AcquireNextImage();
}

}  // namespace fg
