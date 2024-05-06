#pragma once
// #include <vulkan/vulkan.h>
// #include <glm/glm.hpp>
// #include "../Core/Base.h"
// namespace FooGame
// {
//
//     class API
//     {
//         public:
//             void Init();
//             void Draw(u32 vertexCount, u32 instanceCount,
//                       u32 vertexOffset);  // lol
//             void DrawIndexed(u32 indexCount, u32 instanceCount,
//                              u32 firstIndex = 0, u32 firstInstance = 0,
//                              u32 vertexOffset = 0);  // lol
//             VkDevice GetDevice();
//             VkPhysicalDevice GetPhysicalDevice();
//             VkPhysicalDeviceMemoryProperties GetMemoryProperties();
//             void ResizeSwapchain();
//             void SetClearColor(VkClearValue clearVal);
//
//             void SetVertexBuffer(VkBuffer* buffer);
//             void BindIndexBuffer(VkBuffer* buffer);
//
//             void WaitForNewImage();
//             void StartRecording();
//             void StopRecording();
//             void Submit();
//             u32 GetBackBufferIndex() const;
//             void UpdateUniformBuffer(UniformBufferData& data);
//             VkExtent2D GetSwapchainExtent();
//     };
// }  // namespace FooGame
