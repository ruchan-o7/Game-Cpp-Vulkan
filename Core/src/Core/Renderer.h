#pragma once
#include "Render/VulkanContext.h"
namespace FooGame
{
    class Renderer
    {
        public:
            static void Init();
            static void Shutdown();
            static void Resize();
            static void DrawFrame();
            static void BeginDraw();
            static void EndDraw();

            static VkDevice GetDevice();

        private:
            static Context* s_Context;
    };

}  // namespace FooGame
