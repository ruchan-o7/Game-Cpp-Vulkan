#include "VulkanInstance.h"
#include <cassert>
#include <memory>
#include <sstream>
#include <cstring>
#include <Log.h>
#include "Utils/VulkanDebug.h"
#include "vulkan/vulkan_core.h"
namespace ENGINE_NAMESPACE
{

    static constexpr std::array<const char*, 1> ValidationLayerNames = {
        "VK_LAYER_KHRONOS_validation"};
    std::shared_ptr<VulkanInstance> VulkanInstance::Create(const VulkanInstance::CreateInfo& ci)
    {
        auto* instance = new VulkanInstance(ci);
        return std::shared_ptr<VulkanInstance>(instance);
    }
    VulkanInstance::~VulkanInstance()
    {
        if (m_DebugMode)
        {
            FreeDebug(m_VkInstance);
        }
        vkDestroyInstance(m_VkInstance, nullptr);
    }
    VulkanInstance::VulkanInstance(const VulkanInstance::CreateInfo& ci)
        : m_VkInstance(nullptr), m_pAllocCallback(ci.allocCallback)
    {
        {
            // Enumerate available layers
            uint32_t LayerCount = 0;

            auto res = vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);
            if (res != VK_SUCCESS)
            {
                FOO_ENGINE_ERROR("Failed to query layer count");
            }
            m_Layers.resize(LayerCount);
            res = vkEnumerateInstanceLayerProperties(&LayerCount, m_Layers.data());
            if (res != VK_SUCCESS)
            {
                FOO_ENGINE_ERROR("Failed to enumerate extensions");
            }

            assert(LayerCount == m_Layers.size());
        }
        {
            if (ci.printLayers)
            {
                if (!m_Layers.empty())
                {
                    std::stringstream ss;
                    for (const auto& layer : m_Layers)
                    {
                        ss << "\n \t" << layer.layerName << ' ';
                    }
                    FOO_ENGINE_INFO("Available layers {0}", ss.str());
                }
                else
                {
                    FOO_ENGINE_WARN("No vulkan instance layers found");
                }
            }
        }
        if (!EnumerateInstanceExtensions(nullptr, m_Extensions))
        {
            FOO_ENGINE_ERROR("Failed to enumerate instance extensions");
        }
        if (ci.printExtensions)
        {
            if (!m_Extensions.empty())
            {
                FOO_ENGINE_INFO("Supported vulkan instance extensions: \n {0}",
                                PrintExtensionsList(m_Extensions));
            }
            else
            {
                FOO_ENGINE_WARN("No vulkan instance extension found");
            }
        }
        std::vector<const char*> instanceExtensions;
        if (IsExtensionAvailable(m_Extensions, VK_KHR_SURFACE_EXTENSION_NAME))
        {
            instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            instanceExtensions.push_back("VK_KHR_win32_surface");
        }
        if (IsExtensionAvailable(m_Extensions,
                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
        {
            instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        for (const auto* ExtName : instanceExtensions)
        {
            if (!IsExtensionAvailable(m_Extensions, ExtName))
            {
                FOO_ENGINE_ERROR("Required extension {0} is not available", ExtName);
            }
        }
        if (!ci.additionalExtensions.empty())
        {
            for (const auto& ext : ci.additionalExtensions)
            {
                if (IsExtensionAvailable(m_Extensions, ext))
                {
                    instanceExtensions.push_back(ext);
                }
                else
                {
                    FOO_ENGINE_WARN("Requested extension {0} is not available", ext);
                }
            }
        }

        std::vector<const char*> instanceLayers;
        if (!ci.additionalLayers.empty())
        {
            for (const auto& ext : ci.additionalLayers)
            {
                if (IsLayerAvailable(ext))
                {
                    instanceLayers.push_back(ext);
                }
                else
                {
                    FOO_ENGINE_WARN("Requested layer {0} is not available", ext);
                }
            }
        }
        if (ci.enableValidation)
        {
            if (IsExtensionAvailable(m_Extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            for (const auto& layer : ValidationLayerNames)
            {
                if (!IsLayerAvailable(layer))
                {
                    FOO_ENGINE_WARN("Validation layer {0} is not available", layer);
                    continue;
                }
                instanceLayers.push_back(layer);
            }
        }
        VkApplicationInfo appInfo{};
        appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext              = nullptr;
        appInfo.pApplicationName   = nullptr;
        appInfo.applicationVersion = ci.appVersion;
        appInfo.pEngineName        = ci.engineName;
        appInfo.engineVersion      = ci.engineVersion;
        appInfo.apiVersion         = ci.apiVersion;
        VkInstanceCreateInfo instanceCI{};
        instanceCI.sType                 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCI.pNext                 = nullptr;  // Pointer to an extension-specific structure.
        instanceCI.flags                 = 0;
        instanceCI.pApplicationInfo      = &appInfo;
        instanceCI.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        instanceCI.ppEnabledExtensionNames =
            instanceExtensions.empty() ? nullptr : instanceExtensions.data();
        instanceCI.enabledLayerCount   = static_cast<uint32_t>(instanceLayers.size());
        instanceCI.ppEnabledLayerNames = instanceLayers.empty() ? nullptr : instanceLayers.data();

        auto err = vkCreateInstance(&instanceCI, ci.allocCallback, &m_VkInstance);
        if (err != VK_SUCCESS)
        {
            FOO_ENGINE_CRITICAL("Vulkan instance could not created ");
            return;
        }

        m_EnabledExtensions = std::move(instanceExtensions);
        m_VkVersion         = ci.apiVersion;
        m_DebugMode         = ci.debugMode;
        if (ci.debugMode)
        {
            constexpr VkDebugUtilsMessageSeverityFlagsEXT messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            constexpr VkDebugUtilsMessageTypeFlagsEXT messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            if (!SetupDebugUtils(m_VkInstance, messageSeverity, messageType, 0, nullptr))
            {
                FOO_ENGINE_ERROR(
                    "Failed to initialize debug utils. Validation layer message logging, "
                    "performance markers, etc. will be disabled.");
            }
        }
        {
            uint32_t physicalDeviceCount = 0;
            auto err = vkEnumeratePhysicalDevices(m_VkInstance, &physicalDeviceCount, nullptr);
            assert(physicalDeviceCount != 0 && "What the fuck there is no suitable vulkan device");
            m_PhysicalDevices.resize(physicalDeviceCount);
            vkEnumeratePhysicalDevices(m_VkInstance, &physicalDeviceCount,
                                       m_PhysicalDevices.data());
        }
    }
    VkPhysicalDevice VulkanInstance::SelectPhysicalDevice(uint32_t GpuIndex)
    {
        const auto IsGraphicsAndComputeQueueSupported = [](VkPhysicalDevice Device)
        {
            uint32_t QueueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);
            assert(QueueFamilyCount > 0);
            std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QueueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount,
                                                     QueueFamilyProperties.data());
            assert(QueueFamilyCount == QueueFamilyProperties.size());

            // If an implementation exposes any queue family that supports graphics operations,
            // at least one queue family of at least one physical device exposed by the
            // implementation must support both graphics and compute operations.
            for (const auto& QueueFamilyProps : QueueFamilyProperties)
            {
                if ((QueueFamilyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 &&
                    (QueueFamilyProps.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
                {
                    return true;
                }
            }
            return false;
        };

        VkPhysicalDevice SelectedPhysicalDevice = VK_NULL_HANDLE;

        if (GpuIndex < m_PhysicalDevices.size() &&
            IsGraphicsAndComputeQueueSupported(m_PhysicalDevices[GpuIndex]))
        {
            SelectedPhysicalDevice = m_PhysicalDevices[GpuIndex];
        }

        // Select a device that exposes a queue family that supports both compute and graphics
        // operations. Prefer discrete GPU.
        if (SelectedPhysicalDevice == VK_NULL_HANDLE)
        {
            for (auto Device : m_PhysicalDevices)
            {
                VkPhysicalDeviceProperties DeviceProps;
                vkGetPhysicalDeviceProperties(Device, &DeviceProps);

                if (IsGraphicsAndComputeQueueSupported(Device))
                {
                    SelectedPhysicalDevice = Device;
                    if (DeviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                    {
                        break;
                    }
                }
            }
        }

        if (SelectedPhysicalDevice != VK_NULL_HANDLE)
        {
            VkPhysicalDeviceProperties SelectedDeviceProps;
            vkGetPhysicalDeviceProperties(SelectedPhysicalDevice, &SelectedDeviceProps);
            FOO_ENGINE_INFO(
                "Using physical device '{0}', API version: {1}, DriverVersion {2}.{3}.{4} ",
                SelectedDeviceProps.deviceName,
                VK_API_VERSION_MAJOR(SelectedDeviceProps.apiVersion),
                VK_API_VERSION_MINOR(SelectedDeviceProps.apiVersion),
                VK_API_VERSION_PATCH(SelectedDeviceProps.apiVersion),
                VK_API_VERSION_MAJOR(SelectedDeviceProps.driverVersion),
                VK_API_VERSION_MINOR(SelectedDeviceProps.driverVersion),
                VK_API_VERSION_PATCH(SelectedDeviceProps.driverVersion));
        }
        else
        {
            FOO_ENGINE_ERROR("Failed to find suitable physical device");
        }

        return SelectedPhysicalDevice;
    }

    bool EnumerateInstanceExtensions(const char* layerName,
                                     std::vector<VkExtensionProperties>& extensions)
    {
        uint32_t extCount = 0;

        if (vkEnumerateInstanceExtensionProperties(layerName, &extCount, nullptr) != VK_SUCCESS)
        {
            return false;
        }

        extensions.resize(extCount);
        if (vkEnumerateInstanceExtensionProperties(layerName, &extCount, extensions.data()) !=
            VK_SUCCESS)
        {
            extensions.clear();
            return false;
        }
        assert(extCount == extensions.size() &&
               "The number of extensions written by vkEnumerateInstanceExtensionProperties is not "
               "consistent "
               "with the count returned in the first call. This is a Vulkan loader bug.");

        return true;
    }
    std::string PrintExtensionsList(const std::vector<VkExtensionProperties>& extensions);

    bool VulkanInstance::IsLayerAvailable(const char* layer)
    {
        for (const auto& l : m_Layers)
        {
            if (strcmp(l.layerName, layer) == 0)
            {
                return true;
            }
        }
        return false;
    }
    bool VulkanInstance::IsExtensionAvailable(const std::vector<VkExtensionProperties>& extensions,
                                              const char* ext)
    {
        assert(ext != nullptr);

        for (const auto& Extension : extensions)
        {
            if (strcmp(Extension.extensionName, ext) == 0)
            {
                return true;
            }
        }

        return false;
    }
    bool VulkanInstance::IsExtensionEnabled(const char* extension)
    {
        for (const auto& Extension : m_EnabledExtensions)
        {
            if (strcmp(Extension, extension) == 0)
            {
                return true;
            }
        }

        return false;
    }

    std::string PrintExtensionsList(const std::vector<VkExtensionProperties>& extensions)
    {
        std::vector<std::string> ExtStrings;
        ExtStrings.reserve(extensions.size());
        std::stringstream ss;
        for (const auto& Ext : extensions)
        {
            ss << '\t' << Ext.extensionName << ' ' << VK_API_VERSION_MAJOR(Ext.specVersion) << '.'
               << VK_API_VERSION_MINOR(Ext.specVersion) << '.'
               << VK_API_VERSION_PATCH(Ext.specVersion) << '\n';
            ExtStrings.emplace_back(ss.str());
        }

        return ss.str();
    }

}  // namespace ENGINE_NAMESPACE
