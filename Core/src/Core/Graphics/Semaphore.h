#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
namespace FooGame
{
    class Fence
    {
        public:
            Fence(VkDevice* device);

        private:
            VkFence m_Fence;
            VkDevice* m_Device;
    };
    class Semaphore
    {
        public:
            Semaphore(VkDevice* device);

        private:
            VkSemaphore m_Semaphore;
            VkDevice* m_Device;
    };
}  // namespace FooGame
