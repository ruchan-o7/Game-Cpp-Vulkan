#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include "../Camera/PerspectiveCamera.h"
#include "../Camera/OrthographicCamera.h"
namespace FooGame
{
    class Texture2D;
    struct UniformBufferObject;
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
            static void DrawQuad(const glm::vec2& position, const glm::vec2& size,
                                 const glm::vec4& color);

            static void DrawQuad(const glm::vec3& position, const glm::vec2& size,
                                 const glm::vec4& color);

            static void DrawQuad(const glm::vec2& position, const glm::vec2& size,
                                 const std::shared_ptr<Texture2D>& texture,
                                 float tilingFactor         = 1.0f,
                                 const glm::vec4& tintColor = glm::vec4(1.0f));
            static void DrawQuad(const glm::vec3& position, const glm::vec2& size,
                                 const std::shared_ptr<Texture2D>& texture,
                                 float tilingFactor         = 1.0f,
                                 const glm::vec4& tintColor = glm::vec4(1.0f));

            static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);

            static void DrawQuad(const glm::mat4& transform,
                                 const std::shared_ptr<Texture2D>& texture,
                                 float tilingFactor         = 1.0f,
                                 const glm::vec4& tintColor = glm::vec4(1.0f));
            static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size,
                                        float rotation, const glm::vec4& color);
            static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size,
                                        float rotation, const glm::vec4& color);

            struct Statistics
            {
                    uint32_t DrawCalls = 0;
                    uint32_t QuadCount = 0;
                    uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
                    uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
            };
            static void ResetStats();
            static Statistics GetStats();

        private:
            static void BeginRecording(VkCommandBuffer cmd);
            static void BindPipeline(VkCommandBuffer cmd);
            static void NextBatch();
            static void StartBatch();
            static void UpdateUniformData(UniformBufferObject ubd);
            // static void BindDescriptorSets(
            //     VkCommandBuffer cmd, Pipeline& pipeline,
            //     VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            //     uint32_t firstSet = 0, uint32_t dSetCount = 1, uint32_t dynamicOffsetCount = 0,
            //     uint32_t* dynamicOffsets = nullptr);
    };

}  // namespace FooGame
