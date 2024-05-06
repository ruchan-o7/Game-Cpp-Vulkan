#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../Core/Base.h"
namespace FooGame
{
    struct DeviceCreateInfo
    {
            VkInstance pInstance;
            List<const char*> deviceExtensions;
            u32 deviceExtensionCount;
            List<const char*> validationLayers;
            u32 validationLayersCount;
    };
    class Device
    {
        public:
            Device(DeviceCreateInfo info);
            ~Device();

        public:
            VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
            u32 GetGraphicsFamily() const { return m_GraphicQueueFamily; }
            VkQueue GetPresentQueue() const { return m_PresentQueue; }
            VkPhysicalDevice GetPhysicalDevice() const
            {
                return m_PhysicalDevice;
            }
            VkDevice GetDevice() const { return m_Device; }
            VkSurfaceCapabilitiesKHR GetSurfaceCaps(VkSurfaceKHR surface);
            List<VkSurfaceFormatKHR> GetSurfaceFormats(VkSurfaceKHR surface);
            List<VkPresentModeKHR> GetSurfacePresentModes(VkSurfaceKHR surface);
            VkPhysicalDeviceMemoryProperties GetMemoryProperties();

        private:
            VkDevice m_Device;
            VkPhysicalDevice m_PhysicalDevice;
            VkQueue m_PresentQueue;
            VkQueue m_GraphicsQueue;

        private:
            u32 m_PhysicalDeviceCount = 0;
            u32 m_QueueFamilyCount    = -1;
            u32 m_GraphicQueueFamily  = -1;
            u32 m_PresentQueueFamily  = -1;
    };
    class DeviceCreateBuilder
    {
        public:
            DeviceCreateBuilder(VkInstance instance);
            ~DeviceCreateBuilder() = default;
            DeviceCreateBuilder& AddExtension(const char* extension);
            DeviceCreateBuilder& AddLayer(const char* layer);
            [[nodiscard]] Shared<Device> Build();

        private:
            DeviceCreateInfo ci;
    };
}  // namespace FooGame
