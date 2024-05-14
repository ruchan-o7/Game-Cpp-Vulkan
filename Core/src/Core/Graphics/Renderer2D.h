#pragma once
#include "../Core/Base.h"
#include "Core/Graphics/Api.h"
#include "Core/Graphics/Texture2D.h"
#include <vulkan/vulkan.h>
namespace FooGame
{
    class OrthographicCamera;
    class PerspectiveCamera;
    class Renderer2D
    {
        public:
            static void Init();
            static void BeginScene(const OrthographicCamera& camera);
            static void BeginScene(const PerspectiveCamera& camera);
            static void EndScene();
            static void Flush();
            static void Shutdown();
            static void BeginDraw();
            static void EndDraw();

        public:
            static void DrawQuad(const glm::vec2& position,
                                 const glm::vec2& size, const glm::vec4& color);
            static void DrawQuad(const glm::vec3& position,
                                 const glm::vec2& size, const glm::vec4& color);

            static void DrawQuad(const glm::vec2& position,
                                 const glm::vec2& size,
                                 const Shared<Texture2D>& texture,
                                 float tilingFactor         = 1.0f,
                                 const glm::vec4& tintColor = glm::vec4(1.0f));
            static void DrawQuad(const glm::vec3& position,
                                 const glm::vec2& size,
                                 const Shared<Texture2D>& texture,
                                 float tilingFactor         = 1.0f,
                                 const glm::vec4& tintColor = glm::vec4(1.0f));

            static void DrawQuad(const glm::mat4& transform,
                                 const glm::vec4& color);

            static void DrawQuad(const glm::mat4& transform,
                                 const Shared<Texture2D>& texture,
                                 float tilingFactor         = 1.0f,
                                 const glm::vec4& tintColor = glm::vec4(1.0f));
            static void DrawRotatedQuad(const glm::vec2& position,
                                        const glm::vec2& size, float rotation,
                                        const glm::vec4& color);
            static void DrawRotatedQuad(const glm::vec3& position,
                                        const glm::vec2& size, float rotation,
                                        const glm::vec4& color);

            struct Statistics
            {
                    u32 DrawCalls = 0;
                    u32 QuadCount = 0;
                    u32 GetTotalVertexCount() const { return QuadCount * 4; }
                    u32 GetTotalIndexCount() const { return QuadCount * 6; }
            };
            static void ResetStats();
            static Statistics GetStats();

        private:
            static void BeginRecording(VkCommandBuffer cmd);
            static void BindPipeline(VkCommandBuffer cmd);
            static void NextBatch();
            static void StartBatch();
            static void UpdateUniformData(UniformBufferObject ubd);
            static void BindDescriptorSets(
                VkCommandBuffer cmd, Pipeline& pipeline,
                VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                u32 firstSet = 0, u32 dSetCount = 1, u32 dynamicOffsetCount = 0,
                u32* dynamicOffsets = nullptr);
    };

}  // namespace FooGame
