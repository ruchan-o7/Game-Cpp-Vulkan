#pragma once
#include <vulkan/vulkan.h>
#include "../Core/Base.h"
namespace FooGame
{
    class Fence
    {
        public:
            Fence(VkDevice device);
            Fence(const Fence& other) = delete;
            Fence(Fence&& other);
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
            Semaphore(Semaphore&& other);
            Semaphore(const Semaphore& other) = delete;
            Semaphore(VkDevice device);
            VkSemaphore Get() const { return m_Semaphore; }
            void Destroy(VkDevice device);

        private:
            VkSemaphore m_Semaphore;
            VkDevice m_Device;
    };
}  // namespace FooGame
