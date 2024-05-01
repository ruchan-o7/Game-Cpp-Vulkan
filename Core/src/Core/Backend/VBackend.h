#pragma once
#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include "../Core/Base.h"
namespace FooGame
{

    class Context
    {
        public:
            void Init();
            VkInstance GetInstance();
            vkb::Device GetDevice();
            void DrawNotIndexed(u32 vertexCount, u32 instanceCount,
                                u32 vertexOffset);  // lol
            void DrawIndexed(u32 indexCount, u32 instanceCount,
                             u32 firstIndex = 0, u32 firstInstance = 0,
                             u32 vertexOffset = 0);  // lol
            void ResizeSwapchain();
            void BeginDraw();
            void EndDraw();
            void SetClearColor(VkClearValue clearVal);
            void BindVertexBuffer(VkBuffer buffer);
            void BindIndexBuffer(VkBuffer buffer);
    };
}  // namespace FooGame
