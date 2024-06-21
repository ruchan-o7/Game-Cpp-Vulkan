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
            static void EndDraw();
            static void BeginScene(const PerspectiveCamera& camera);
            static void BeginScene(const Camera& camera);
            static void BeginScene(const glm::mat4& view, const glm::mat4& projection);
            static void Shutdown();
            static void ClearBuffers();
            static FrameStatistics GetStats();

        public:
            static void DrawModel(Model* model, const glm::mat4& transform);
            static void DrawModel(const std::string& name, const glm::mat4& transform);
            static void DrawMesh(const std::string& name, glm::mat4& transform);

        public:
            static void SubmitMesh(Mesh& mesh);
            static void SubmitMesh(Mesh* mesh);
            static void SubmitModel(Model* model);
            static void SubmitModel(std::shared_ptr<Model> model);
            static void SubmitModel(const std::string& name);

        private:
            static void BindPipeline(VkCommandBuffer cmd);
            static void UpdateUniformData(UniformBufferObject& ubd);
    };
}  // namespace FooGame
