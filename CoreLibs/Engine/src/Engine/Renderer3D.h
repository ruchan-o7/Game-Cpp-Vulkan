#pragma once
#include "../Geometry/Model.h"
#include "../Engine/Types/GraphicTypes.h"
namespace FooGame
{

    class PerspectiveCamera;
    class Camera;
    struct Pipeline;
    struct FrameStatistics
    {
            uint32_t DrawCall    = 0;
            uint64_t VertexCount = 0;
            uint64_t IndexCount  = 0;
    };
    class Renderer3D
    {
        public:
            static void Init(class RenderDevice* pRenderDevice);
            static void BeginDraw();
            static void EndDraw();
            static void BeginScene(const PerspectiveCamera& camera);
            static void BeginScene(const Camera& camera);
            static void EndScene();
            static void Shutdown();
            static void ClearBuffers();
            static FrameStatistics GetStats();

        public:
            static void DrawModel(uint32_t id, const glm::mat4& transform);
            // static void DrawMesh(uint32_t id, const glm::mat4& transform, const Texture2D&
            // texture);
            static void DrawModel(Model* model, const glm::mat4& transform);

        public:
            static void SubmitMesh(Mesh* mesh);
            static void SubmitModel(Model* model);

        private:
            static void BindPipeline(VkCommandBuffer cmd);
            static void UpdateUniformData(UniformBufferObject& ubd);
            // static void BindDescriptorSets(
            //     VkCommandBuffer cmd, const Texture2D& texture, VkDescriptorSet& set,
            //     VkPipelineLayout layout,
            //     VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            //     uint32_t firstSet = 0, uint32_t dSetCount = 1, uint32_t dynamicOffsetCount = 0,
            //     uint32_t* dynamicOffsets = nullptr);
            static void BindDescriptorSets(
                VkCommandBuffer cmd, Mesh& mesh, VkPipelineLayout layout,
                VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                uint32_t firstSet = 0, uint32_t dSetCount = 1, uint32_t dynamicOffsetCount = 0,
                uint32_t* dynamicOffsets = nullptr);
    };
}  // namespace FooGame
