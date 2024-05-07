#pragma once

#include <vulkan/vulkan.h>
#include "../Core/Base.h"
#include "../Graphics/Api.h"
#include "../Graphics/Buffer.h"
#include "../Graphics/Semaphore.h"
#include "../Graphics/Swapchain.h"
#include "vulkan/vulkan_core.h"
struct GLFWwindow;
namespace FooGame
{
    struct FrameData
    {
            // u32 imageIndex   = 0;
            u32 currentFrame = 0;
    };
    class Engine
    {
        public:
            Engine() = default;
            ~Engine();
            static Engine* Create(GLFWwindow* window);
            static Engine* Get() { return s_Instance; }
            void Init(GLFWwindow* window);
            void Shutdown();
            void RunLoop();
            void Close() { m_ShouldClose = true; }
            void Submit(u32 imageIndex);
            VkCommandBuffer BeginSingleTimeCommands();
            void EndSingleTimeCommands(VkCommandBuffer& commandBuffer);
            Device& GetDevice() const;

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
            Image m_Image;
            VkSampler m_TextureSampler;

        private:
            static Engine* s_Instance;
            bool ShouldClose() const { return m_ShouldClose; }
            void WaitFences();
            void ResetFences();
            void ResetCommandBuffers();
            void RecreateSwapchain();
            bool AcquireNextImage(u32& imageIndex);
            void UpdateUniforms();
            void Record(const u32& imageIndex);
    };
}  // namespace FooGame
