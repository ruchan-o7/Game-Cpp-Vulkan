#include "Renderer.h"
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#include <memory>
#include <stdexcept>
#include "src/Core/Log.h"
#include "src/Renderer/VulkanInstance.h"
#include "src/Renderer/VulkanPhysicalDevice.h"
#include "vulkan/vulkan_core.h"

namespace fg {

const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation",
};

VkDebugUtilsMessengerEXT s_DebugMessenger;

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  if (messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    FOO_CORE_INFO("[VULKAN]: {}", pCallbackData->pMessage);
  } else {
    FOO_CORE_ERROR("[VULKAN]: {}", pCallbackData->pMessage);
  }

  return VK_FALSE;
}

std::shared_ptr<Renderer> Renderer::Create(GLFWwindow* window, const VkAllocationCallbacks* acb) {
  VkInstance vkInstance = VK_NULL_HANDLE;
  VkPhysicalDevice vkPDevice = VK_NULL_HANDLE;

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

  VkDebugUtilsMessengerCreateInfoEXT messenger {
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
  messenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  messenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  messenger.pfnUserCallback = debugCallback;
  pCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messenger;

  VkResult res = vkCreateInstance(&pCreateInfo, acb, &vkInstance);

  if (res != VK_SUCCESS) {
    throw std::runtime_error("Can not create vulkan instance");
  }

  auto Instance = std::make_shared<VulkanInstance>(vkInstance, acb);

  res = CreateDebugUtilsMessengerEXT(vkInstance, &messenger, acb, &s_DebugMessenger);

  VkPhysicalDevice physicalDevices[8];
  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
  vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices);

  for (uint32_t i = 0; i < physicalDeviceCount; i++) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &props);
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      vkPDevice = physicalDevices[i];
    }
  }
  auto physicalDevice = std::make_unique<VulkanPhysicalDevice>(vkPDevice);

  auto renderer = new Renderer(window, Instance, std::move(physicalDevice), acb);

  return std::shared_ptr<Renderer>(renderer);
}

Renderer::Renderer(GLFWwindow* window, const std::shared_ptr<VulkanInstance>& instance,
                   std::unique_ptr<VulkanPhysicalDevice> pDevice,
                   const VkAllocationCallbacks* alloc)
    : m_Instance(instance),
      m_Window(window),
      m_PhysicalDevice(std::move(pDevice)),
      m_AllocCB(alloc) {
  uint32_t queueIndex = m_PhysicalDevice->GetQueuFamilyIndices(VK_QUEUE_GRAPHICS_BIT);

  VkDeviceQueueCreateInfo queueInfo {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  queueInfo.queueFamilyIndex = queueIndex;
  queueInfo.queueCount = 1;
  float priority[] = {1.0f};
  queueInfo.pQueuePriorities = priority;

  auto pDev = m_PhysicalDevice->GetHandle();

  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(pDev, &features);

  const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDeviceCreateInfo info {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  info.pQueueCreateInfos = &queueInfo;
  info.queueCreateInfoCount = 1;
  info.pEnabledFeatures = &features;
  info.enabledExtensionCount = 1;
  info.ppEnabledExtensionNames = extensions;
  info.enabledLayerCount = 1;
  info.ppEnabledLayerNames = validationLayers;

  VkDevice device = VK_NULL_HANDLE;
  VkResult res = vkCreateDevice(pDev, &info, m_AllocCB, &device);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Can not create logical device");
  }
  m_LogicalDevice = std::make_shared<VulkanLogicalDevice>(device, queueIndex, m_AllocCB);
  m_Swapchain =
      std::make_shared<VulkanSwapchain>(m_Window, m_Instance, m_LogicalDevice, *m_PhysicalDevice);
}

void Renderer::Destroy() {
}

}  // namespace fg
