#include "Renderer.h"
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#include <fstream>
#include <ios>
#include <memory>
#include <stdexcept>
#include "src/Core/Assert.h"
#include "src/Core/Log.h"
#include "src/Renderer/VulkanDebug.h"
#include "src/Renderer/VulkanInstance.h"
#include "src/Renderer/VulkanPhysicalDevice.h"
#include "src/Renderer/VulkanShader.h"
#include "vulkan/vulkan_core.h"

namespace fg {

const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation",
};

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

  VkResult res = vkCreateInstance(&pCreateInfo, acb, &vkInstance);

  if (res != VK_SUCCESS) {
    throw std::runtime_error("Can not create vulkan instance");
  }

  auto Instance = std::make_shared<VulkanInstance>(vkInstance, acb);

  auto messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
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

  for (uint32_t i = 0; i < physicalDeviceCount; i++) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &props);
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      vkPDevice = physicalDevices[i];
    }
  }
  auto physicalDevice = std::make_unique<VulkanPhysicalDevice>(vkPDevice);

  Renderer* renderer = new Renderer(window, Instance, std::move(physicalDevice), acb);

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

std::shared_ptr<VulkanShader> Renderer::CreateShader(const ShaderDescription& desc) {
  FOO_ASSERT(desc.Stage != 0)
  Buffer buff;
  if (desc.ByteCode) {
    buff = desc.ByteCode;
  } else {
    std::ifstream in {desc.Path, std::ios::ate | std::ios::binary};
    if (!std::filesystem::exists(desc.Path)) {
      auto formatted = fmt::format("Can not find: '{}' does not exists!", desc.Path.string());
      FOO_CORE_ERROR(formatted);
      return nullptr;
    }
    if (!in.is_open()) {
      FOO_CORE_ERROR("Can not open file: {}", desc.Path.string());
      return nullptr;
    }
    size_t size = in.tellg();
    if (size == 0) {
      FOO_CORE_ERROR("Failed to read: '{}' does not exists!", desc.Path.string());
      return nullptr;
    }
    in.seekg(0);
    buff.Allocate(size);
    in.read(buff.As<char>(), size);
  }
  VkShaderModuleCreateInfo info {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  info.pCode = buff.As<uint32_t>();
  info.codeSize = buff.Size;
  auto handle = m_LogicalDevice->CreateShader(info, desc.Name);
  if (handle) {
    return std::make_shared<VulkanShader>(std::move(handle), desc.Stage);
  }

  FOO_CORE_ERROR("Can not create shader handle: Name: {}", desc.Name != nullptr ? desc.Name : "");
  return nullptr;
}

void Renderer::Destroy() {
}

}  // namespace fg
