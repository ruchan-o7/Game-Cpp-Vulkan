#pragma once
#include <vulkan/vulkan.h>
#include "../Core/Base.h"
#include "Device.h"
namespace FooGame
{
    class Command
    {
        public:
            Command(const Device& device, u32 commandBufferCount);
            ~Command();
            VkCommandBuffer Get(u32 index) const
            {
                return m_CommandBuffers[index];
            }
            void Reset(u32 index, VkCommandBufferResetFlags flags = 0);
            void StartRecording(u32 index);
            void StopRecording(u32 index);

        private:
            VkCommandPool m_CommandPool;
            List<VkCommandBuffer> m_CommandBuffers;
    };
}  // namespace FooGame
