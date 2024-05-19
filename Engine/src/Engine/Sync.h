#pragma once
#include <vulkan/vulkan.h>
#include "../Defines.h"
namespace FooGame
{
    class Fence
    {
            DELETE_COPY(Fence);

        public:
            Fence(VkDevice device);
            Fence(Fence&& other);
            void Wait(uint64_t timeOut = UINT64_MAX);
            VkFence Get() const { return m_Fence; }
            void Reset();
            void Destroy(VkDevice device);

        private:
            VkFence m_Fence;
            VkDevice m_Device;
    };
    class Semaphore
    {
            DELETE_COPY(Semaphore);

        public:
            Semaphore(Semaphore&& other);
            Semaphore(VkDevice device);
            VkSemaphore Get() const { return m_Semaphore; }
            void Destroy(VkDevice device);

        private:
            VkSemaphore m_Semaphore;
            VkDevice m_Device;
    };
}  // namespace FooGame
