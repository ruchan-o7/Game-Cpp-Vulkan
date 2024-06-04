#pragma once
#include "../Defines.h"
#include <string>
#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
namespace ENGINE_NAMESPACE
{

    class VulkanInstance : public std::enable_shared_from_this<VulkanInstance>
    {
        public:
            struct CreateInfo
            {
                    const char* appName    = "Foo game";
                    const char* engineName = "Foo renderer";
                    uint32_t apiVersion    = VK_API_VERSION_1_3;
                    uint32_t appVersion    = VK_MAKE_VERSION(0, 1, 0);
                    uint32_t engineVersion = VK_MAKE_VERSION(0, 1, 0);
                    bool printLayers       = true;
                    bool printExtensions   = true;
                    bool enableValidation  = true;
                    std::vector<const char*> additionalExtensions;
                    std::vector<const char*> additionalLayers;
                    VkAllocationCallbacks* allocCallback = nullptr;
                    bool debugMode                       = true;
            };
            static std::shared_ptr<VulkanInstance> Create(const CreateInfo& ci);
            ~VulkanInstance();
            DELETE_COPY_MOVE(VulkanInstance);
            std::shared_ptr<VulkanInstance> GetSharedPtr() { return shared_from_this(); }
            std::shared_ptr<const VulkanInstance> GetSharedPtr() const
            {
                return shared_from_this();
            }
            VkAllocationCallbacks* GetVkAllocator() const { return m_pAllocCallback; }
            bool IsLayerAvailable(const char* layer);
            bool IsExtensionAvailable(const std::vector<VkExtensionProperties>& extensions,
                                      const char* ext);
            bool IsExtensionEnabled(const char* extension);
            VkPhysicalDevice SelectPhysicalDevice(uint32_t GpuIndex);
            const std::vector<VkPhysicalDevice>& GetPhysicalDevices() const
            {
                return m_PhysicalDevices;
            }

        private:
            explicit VulkanInstance(const CreateInfo& ci);

        private:
            VkInstance m_VkInstance;
            VkAllocationCallbacks* const m_pAllocCallback;
            std::vector<const char*> m_EnabledExtensions;
            std::vector<VkExtensionProperties> m_Extensions;
            std::vector<VkLayerProperties> m_Layers;
            std::vector<VkPhysicalDevice> m_PhysicalDevices;
            bool m_DebugMode;
            uint32_t m_VkVersion;
    };
    bool EnumerateInstanceExtensions(const char* layerName,
                                     std::vector<VkExtensionProperties>& Extensions);
    std::string PrintExtensionsList(const std::vector<VkExtensionProperties>& extensions);

}  // namespace ENGINE_NAMESPACE
