#pragma once
#include <vulkan/vulkan.h>
#include "../Defines.h"
namespace Engine
{
    struct DeviceCreateInfo
    {
            List<const char*> deviceExtensions;
            List<const char*> validationLayers;
    };
    class Device
    {
        public:
            static Device* CreateDevice(const List<const char*>& layers,
                                        const List<const char*>& extensions);

        public:
            VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
            u32 GetGraphicsFamily() const { return m_GraphicQueueFamily; }
            VkQueue GetPresentQueue() const { return m_PresentQueue; }
            VkPhysicalDevice GetPhysicalDevice() const
            {
                return m_PhysicalDevice;
            }
            VkDevice GetDevice() const { return m_Device; }
            VkPhysicalDeviceProperties GetPhysicalDeviceProperties();
            VkSurfaceCapabilitiesKHR GetSurfaceCaps(VkSurfaceKHR surface);
            List<VkSurfaceFormatKHR> GetSurfaceFormats(VkSurfaceKHR surface);
            List<VkPresentModeKHR> GetSurfacePresentModes(VkSurfaceKHR surface);
            VkPhysicalDeviceMemoryProperties GetMemoryProperties();
            VkMemoryRequirements GetMemoryRequirements(VkImage& image);
            VkMemoryRequirements GetMemoryRequirements(VkBuffer& buffer);
            void AllocateMemory(VkMemoryAllocateInfo& allocInfo,
                                VkDeviceMemory& memory);

            u32 FindMemoryType(u32 filter, VkMemoryPropertyFlags properties);
            void WaitIdle();

        private:
            Device(DeviceCreateInfo info);
            ~Device() = default;
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
}  // namespace Engine
