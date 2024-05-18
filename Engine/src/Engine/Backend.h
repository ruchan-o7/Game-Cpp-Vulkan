#pragma once

#include <vulkan/vulkan.h>
#include "../Window/Window.h"
#include "Api.h"
#include "Sync.h"
#include "../Events/ApplicationEvent.h"
namespace Engine
{
    class Backend
    {
        public:
            ~Backend();
            static void Init(WindowsWindow& window);
            static void BeginDrawing();
            static void EndDrawing();
            static void WaitFence(Fence& fence);
            static void Shutdown();
            static void UpdateUniformData(UniformBufferObject ubo);
            static void ResetCommandBuffer(VkCommandBuffer& buf,
                                           VkCommandBufferResetFlags flags = 0);
            static VkCommandBuffer BeginSingleTimeCommands();
            static void EndSingleTimeCommands(VkCommandBuffer& commandBuffer);
            static void BeginRenderpass(VkRenderPass& renderpass);

            static bool OnWindowResized(WindowResizeEvent& event);
            static u32 GetCurrentFrame();
            static u32 GetImageIndex();
            static VkFormat GetSwapchainImageFormat();
            static VkExtent2D GetSwapchainExtent();
            static VkRenderPass GetRenderPass();
            static VkFramebuffer GetFramebuffer();
            static VkCommandBuffer GetCurrentCommandbuffer();
            static void BeginRenderpass();

        private:
            static void BeginDrawing_();
            static void Submit();
            static void InitImgui();
            static void RecreateSwapchain();
            static bool AcquireNextImage(u32& imageIndex);
    };
}  // namespace Engine
