#pragma once

#include "Api.h"
namespace FooGame
{
    class Window;
    class WindowResizeEvent;
    class RenderDevice;
    class VulkanBuffer;
    class VulkanTexture;
    class Backend
    {
        public:
            ~Backend();
            static void Init(Window& window);
            static void EndDrawing();
            static void WaitIdle();
            static void Shutdown();
            static VkCommandBuffer BeginSingleTimeCommands();
            static void EndSingleTimeCommands(VkCommandBuffer& commandBuffer);
            static void BeginRenderpass(VkRenderPass& renderpass);

            static bool OnWindowResized(WindowResizeEvent& event);
            static uint32_t GetCurrentFrame();
            static uint32_t GetImageIndex();
            static VkFormat GetSwapchainImageFormat();
            static VkExtent2D GetSwapchainExtent();
            static VkRenderPass GetRenderPass();
            static VkFramebuffer GetFramebuffer();
            static VkCommandBuffer GetCurrentCommandbuffer();
            static void BeginRenderpass();
            static VkCommandPool GetCommandPool();

            static RenderDevice* GetRenderDevice();

            static void TransitionImageLayout(class VulkanImage* image, VkFormat format,
                                              VkImageLayout oldLayout, VkImageLayout newLayout);
            static void CopyBufferToImage(VulkanBuffer& source, VulkanTexture& destination);

        private:
            static void BeginDrawing_();
            static void Submit();
            static void InitImgui();
    };
}  // namespace FooGame
