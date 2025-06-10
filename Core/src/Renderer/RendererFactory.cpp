#include "RendererFactory.h"
#include "VulkanInstance.h"
#include "VulkanLogicalDevice.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanDebug.h"
#include "RenderContext.h"
#include "Renderer.h"

#include "../Core/Assert.h"
#include "src/Core/Ref.h"
#include "src/Renderer/VulkanQueue.h"

#include <GLFW/glfw3.h>
#include <utility>

namespace fg {

const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation",
};

RendererFactory& RendererFactory::Get() {
  static RendererFactory factory;
  return factory;
}

void RendererFactory::CreateDeviceAndContexts(const EngineInfo& info, Ref<Renderer>* renderer,
                                              Ref<RenderContext>* context) {
  FOO_ASSERT(renderer != nullptr);
  FOO_ASSERT(context != nullptr);
  FOO_ASSERT(*context == nullptr);
  if (auto renderer = m_wRenderer.Lock()) {
    FOO_CORE_ERROR("Can not have more than renderer instance");
    return;
  }

  VkInstance vkInstance = VK_NULL_HANDLE;

  VkApplicationInfo appInfo {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.pApplicationName = "Foo game engine";
  appInfo.pEngineName = "FG Engine";
  appInfo.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo pCreateInfo {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  pCreateInfo.pApplicationInfo = &appInfo;

  uint32_t extCount = 0;
  const char** glfwExtensions = nullptr;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&extCount);
  std::vector<const char*> instanceExtensions;
  instanceExtensions.reserve(extCount + 1);
  for (int i = 0; i < extCount; i++) {
    instanceExtensions.emplace_back(glfwExtensions[i]);
  }
  instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  pCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
  pCreateInfo.enabledExtensionCount = instanceExtensions.size();
  pCreateInfo.ppEnabledLayerNames = validationLayers;
  pCreateInfo.enabledLayerCount = 1;

  VkResult res = vkCreateInstance(&pCreateInfo, info.Allocator, &vkInstance);
  FOO_ASSERT(res == VK_SUCCESS,
             "Can not create vulkan instance, maybe device is not supports vulkan");

  volkLoadInstance(vkInstance);

  if (res != VK_SUCCESS) {
    throw std::runtime_error("Can not create vulkan instance");
  }

  auto Instance = std::make_shared<VulkanInstance>(vkInstance, info.Allocator);

  auto messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  auto messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  SetupDebugUtils(vkInstance, messageSeverity, messageType, 0, nullptr);

  VkPhysicalDevice physicalDevices[8];
  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
  vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices);
  if (physicalDeviceCount == 0) {
    throw std::runtime_error("Can not supported physical GPU device");
  }
  VkPhysicalDevice selected = VK_NULL_HANDLE;
  for (uint32_t i = 0; i < physicalDeviceCount; i++) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &props);
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      selected = physicalDevices[i];
      break;
    }
  }
  if (selected == VK_NULL_HANDLE) {
    FOO_CORE_INFO("Can not find discrete GPU, selecting first one");
    selected = physicalDevices[0];
  }
  VkPhysicalDeviceProperties pDeviceProps {};
  vkGetPhysicalDeviceProperties(selected, &pDeviceProps);
  FOO_CORE_INFO("Selected GPU: {}", pDeviceProps.deviceName);

  auto physicalDevice = std::make_unique<VulkanPhysicalDevice>(selected);
  auto pDev = physicalDevice->GetHandle();

  uint32_t queueIndex = 0;
  try {
    queueIndex = physicalDevice->GetQueuFamilyIndices(VK_QUEUE_GRAPHICS_BIT);
  } catch (const std::runtime_error& err) {
    FOO_CORE_ERROR("Can't find suitable queue family. Aborting!");
    exit(1);
  }

  float priority[] = {1.0f};

  VkDeviceQueueCreateInfo queueInfo {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  queueInfo.queueFamilyIndex = queueIndex;
  queueInfo.queueCount = 1;
  queueInfo.pQueuePriorities = priority;
  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(pDev, &features);

  const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                              VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};

  VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRendering {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
      VK_NULL_HANDLE,
      VK_TRUE,
  };

  VkDeviceCreateInfo devInfo {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  devInfo.pQueueCreateInfos = &queueInfo;
  devInfo.queueCreateInfoCount = 1;
  devInfo.pEnabledFeatures = &features;
  devInfo.enabledExtensionCount = 2;
  devInfo.ppEnabledExtensionNames = extensions;
  devInfo.enabledLayerCount = 1;
  devInfo.ppEnabledLayerNames = validationLayers;
  devInfo.pNext = &dynamicRendering;

  auto logicalDevice =
      std::make_shared<VulkanLogicalDevice>(devInfo, queueIndex, *physicalDevice, info);

  RenderContextInfo ctxInfo;
  ctxInfo.Id = queueIndex;
  ctxInfo.Name = "Graphics render context";
  Ref<VulkanQueue> queue = MakeRef<VulkanQueue>(logicalDevice, ctxInfo, queueIndex);

  VmaAllocatorCreateInfo allocatorInfo {};
  allocatorInfo.device = logicalDevice->GetHandle();
  allocatorInfo.physicalDevice = physicalDevice->GetHandle();
  allocatorInfo.instance = vkInstance;
  allocatorInfo.vulkanApiVersion = appInfo.apiVersion;

  VmaVulkanFunctions vmaFunc {};
  vmaImportVulkanFunctionsFromVolk(&allocatorInfo, &vmaFunc);
  allocatorInfo.pVulkanFunctions = &vmaFunc;

  VmaAllocator vma = nullptr;
  res = vmaCreateAllocator(&allocatorInfo, &vma);
  if (res != VK_SUCCESS) {
    FOO_CORE_ERROR("Can not initialize VMA");
  } else {
    logicalDevice->SetVMAInstance(vma);
  }
  AttachDevices(Instance, std::move(physicalDevice), logicalDevice, info, queue, renderer, context);
}
void RendererFactory::AttachDevices(std::shared_ptr<VulkanInstance> vkInstance,
                                    std::unique_ptr<VulkanPhysicalDevice> physicalDevice,
                                    std::shared_ptr<VulkanLogicalDevice> logicalDevice,
                                    const EngineInfo& info, Ref<VulkanQueue> queue,
                                    Ref<Renderer>* renderer, Ref<RenderContext>* context) {
  FOO_ASSERT(renderer != nullptr);
  FOO_ASSERT(*renderer == nullptr);
  FOO_ASSERT(context != nullptr);
  FOO_ASSERT(*context == nullptr);
  *renderer =
      MakeRef<Renderer>(this, info, vkInstance, std::move(physicalDevice), logicalDevice, queue);
  auto& pDevice = (*renderer)->GetPhysicalDevice();
  const auto& queueProps = pDevice.GetQueueProps();
  const auto queueType = queueProps[queue->GetFamilyIndex()];

  *context = MakeRef<RenderContext>(
      renderer->get(), info, RenderContextInfo {"Graphics Context", 0, queueType.queueFlags});
  (*renderer)->SetContext(*context);
}
Ref<VulkanSwapchain> RendererFactory::CreateSwapchain(Ref<Renderer> renderer,
                                                      Ref<RenderContext> ctx, GLFWwindow* window) {
  return MakeRef<VulkanSwapchain>(window, renderer, ctx);
}

}  // namespace fg
