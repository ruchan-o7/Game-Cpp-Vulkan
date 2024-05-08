#pragma once

#include <vulkan/vulkan.h>
#include "../Core/Base.h"
#include "../Graphics/Api.h"
#include "../Graphics/Buffer.h"
#include "../Graphics/Semaphore.h"
#include "../Graphics/Swapchain.h"
#include "Core/Core/PerspectiveCamera.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Graphics/Camera.h"
#include "vulkan/vulkan_core.h"
struct GLFWwindow;
namespace FooGame
{
    struct FrameData
    {
            u32 imageIndex               = 0;
            u32 currentFrame             = 0;
            i32 fbWidth                  = 0;
            i32 fbHeight                 = 0;
            static const u32 MaxQuads    = 20000;
            static const u32 MaxVertices = MaxQuads * 4;
            static const u32 MaxIndices  = MaxQuads * 6;
            Vertex* QuadVertexBufferBase = nullptr;
            Vertex* QuadVertexBufferPtr  = nullptr;
            glm::vec4 QuadVertexPositions[4];
            u32 QuadIndexCount = 0;
            u32 QuadCount      = 0;
            u32 DrawCall       = 0;
    };
    class Engine
    {
            using EventCallback = std::function<void(Event&)>;

        public:
            Engine() = default;
            ~Engine();
            static Engine* Create(GLFWwindow* window);
            static Engine* Get() { return s_Instance; }
            void Init(GLFWwindow* window);
            void Start();
            void End();
            void BeginScene(const PerspectiveCamera& camera);
            void EndScene();
            void Flush();
            void Shutdown();
            void Close();
            VkCommandBuffer BeginSingleTimeCommands();
            void EndSingleTimeCommands(VkCommandBuffer& commandBuffer);
            Device& GetDevice() const;

            bool OnWindowResized(WindowResizeEvent& event);

        public:
            // Primitives
            void DrawQuad(const glm::vec2& position, const glm::vec2& size,
                          const glm::vec4& color);
            void DrawQuad(const glm::vec3& position, const glm::vec2& size,
                          const glm::vec4& color);

            void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
            void DrawRotatedQuad(const glm::vec2& position,
                                 const glm::vec2& size, float rotation,
                                 const glm::vec4& color);
            void DrawRotatedQuad(const glm::vec3& position,
                                 const glm::vec2& size, float rotation,
                                 const glm::vec4& color);

        private:
            void Submit();
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
            bool m_FramebufferResized = false;

        private:
            static Engine* s_Instance;
            bool ShouldClose() const { return m_ShouldClose; }
            void WaitFences();
            void ResetFences();
            void ResetCommandBuffers();
            void RecreateSwapchain();
            bool AcquireNextImage(u32& imageIndex);
            void UpdateUniforms(const PerspectiveCamera& camera);
            // void Record();
            void NextBatch();
            void StartBatch();
            void BeginDrawing();
    };
}  // namespace FooGame
