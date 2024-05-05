#pragma once
#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include "../Core/Base.h"
namespace FooGame
{

    struct UniformBufferData
    {
            alignas(16) glm::mat4 Model;
            alignas(16) glm::mat4 View;
            alignas(16) glm::mat4 Projection;
    };
    class API
    {
        public:
            void Init();
            void Draw(u32 vertexCount, u32 instanceCount,
                      u32 vertexOffset);  // lol
            void DrawIndexed(u32 indexCount, u32 instanceCount,
                             u32 firstIndex = 0, u32 firstInstance = 0,
                             u32 vertexOffset = 0);  // lol
            VkDevice GetDevice();
            VkPhysicalDevice GetPhysicalDevice();
            VkPhysicalDeviceMemoryProperties GetMemoryProperties();
            void ResizeSwapchain();
            void SetClearColor(VkClearValue clearVal);

            void SetVertexBuffer(VkBuffer* buffer);
            void BindIndexBuffer(VkBuffer* buffer);

            void WaitForNewImage();
            void StartRecording();
            void StopRecording();
            void Submit();
            u32 GetBackBufferIndex() const;
            void UpdateUniformBuffer(UniformBufferData& data);
            VkExtent2D GetSwapchainExtent();
    };
}  // namespace FooGame
