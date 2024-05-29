#pragma once
#include <vulkan/vulkan.h>
#include <vector>
namespace FooGame
{
    struct DeviceCreateInfo
    {
            std::vector<const char*> deviceExtensions;
            std::vector<const char*> validationLayers;
    };
    class Device
    {
        public:
            static Device* CreateDevice(
                const std::vector<const char*>& layers,
                const std::vector<const char*>& extensions);

        public:
            VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
            uint32_t GetGraphicsFamily() const { return m_GraphicQueueFamily; }
            VkQueue GetPresentQueue() const { return m_PresentQueue; }
            VkPhysicalDevice GetPhysicalDevice() const
            {
                return m_PhysicalDevice;
            }
            VkDevice GetDevice() const { return m_Device; }
            VkPhysicalDeviceProperties GetPhysicalDeviceProperties();
            VkSurfaceCapabilitiesKHR GetSurfaceCaps(VkSurfaceKHR surface);
            std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(
                VkSurfaceKHR surface);
            std::vector<VkPresentModeKHR> GetSurfacePresentModes(
                VkSurfaceKHR surface);
            VkPhysicalDeviceMemoryProperties GetMemoryProperties();
            VkMemoryRequirements GetMemoryRequirements(VkImage& image);
            VkMemoryRequirements GetMemoryRequirements(VkBuffer& buffer);
            void AllocateMemory(VkMemoryAllocateInfo& allocInfo,
                                VkDeviceMemory& memory);

            uint32_t FindMemoryType(uint32_t filter,
                                    VkMemoryPropertyFlags properties);
            void WaitIdle();
            void Destroy();

        private:
            Device(DeviceCreateInfo info);
            ~Device() = default;
            VkDevice m_Device;
            VkPhysicalDevice m_PhysicalDevice;
            VkQueue m_PresentQueue;
            VkQueue m_GraphicsQueue;

        private:
            uint32_t m_PhysicalDeviceCount = 0;
            uint32_t m_QueueFamilyCount    = -1;
            uint32_t m_GraphicQueueFamily  = -1;
            uint32_t m_PresentQueueFamily  = -1;
    };
}  // namespace FooGame
