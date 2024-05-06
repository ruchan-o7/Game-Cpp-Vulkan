#pragma once

#include <vulkan/vulkan.h>
#include "../Core/Base.h"
#include "../Graphics/Api.h"
#include "../Graphics/Buffer.h"
#include "../Graphics/Semaphore.h"
#include "../Graphics/Swapchain.h"
struct GLFWwindow;
namespace FooGame
{
    struct FrameData
    {
            u32 imageIndex = 0;
    };
    class Engine
    {
        public:
            Engine() = default;
            ~Engine();
            void Init(GLFWwindow* window);
            void Shutdown();
            void RunLoop();
            void Close() { m_ShouldClose = true; }
            void Submit();

        private:
            bool m_ShouldClose;
            GLFWwindow* m_WindowHandle;
            Api* m_Api;
            Shared<Buffer> m_VertexBuffer;
            Shared<Buffer> m_IndexBuffer;
            List<Shared<Buffer>> m_UniformBuffers;
            Shared<Swapchain> m_Swapchain;
            List<VkDescriptorSet> m_DescriptorSets;
            List<VkCommandBuffer> m_CommandBuffers;
            List<Fence> m_InFlightFences;
            List<Semaphore> m_ImageAvailableSemaphores;
            List<Semaphore> m_RenderFinishedSemaphores;
            FrameData frameData;

        private:
            bool ShouldClose() const { return m_ShouldClose; }
            void WaitFences();
            void ResetFences();
            void ResetCommandBuffers();
            void RecreateSwapchain();
            bool AcquireNextImage();
            void UpdateUniforms();
            void Record();
    };
}  // namespace FooGame
