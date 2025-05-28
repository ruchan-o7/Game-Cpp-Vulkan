#pragma once
#include "../Engine/Types/GraphicTypes.h"
#include "../../Core/UUID.h"
#include "src/Engine/Core/VulkanPipeline.h"
struct VkCommandBuffer_T;
typedef VkCommandBuffer_T* VkCommandBuffer;
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
            static void EndDraw();
            static void BeginScene(const PerspectiveCamera& camera);
            static void BeginScene(const Camera& camera);
            static void BeginScene(const glm::mat4& view, const glm::mat4& projection);
            static void Shutdown();
            static void ClearBuffers();
            static FrameStatistics GetStats();

        public:
            static void DrawModel(UUID id, const glm::mat4& transform);
            static VulkanPipeline* GetPipeline();

        public:
            static void SubmitModel(UUID id);

        private:
            static void BindPipeline(VkCommandBuffer cmd);
            static void UpdateUniformData(UniformBufferObject& ubd);
    };
}  // namespace FooGame
