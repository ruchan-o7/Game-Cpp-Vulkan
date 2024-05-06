#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../Core/Base.h"
namespace FooGame
{
    class Fence
    {
        public:
            Fence(VkDevice device);
            void Wait(u64 timeOut = UINT64_MAX);
            VkFence Get() const { return m_Fence; }
            void Reset();
            void Destroy(VkDevice device);

        private:
            VkFence m_Fence;
            VkDevice m_Device;
    };
    class Semaphore
    {
        public:
            Semaphore(VkDevice device);
            VkSemaphore Get() const { return m_Semaphore; }
            void Destroy(VkDevice device);

        private:
            VkSemaphore m_Semaphore;
            VkDevice m_Device;
    };
}  // namespace FooGame
