#pragma once
#include <vulkan/vulkan.h>
#include "../Camera/PerspectiveCamera.h"
#include "../Engine/Api.h"
#include "../Engine/Pipeline.h"
#include "../Geometry/Model.h"
namespace FooGame
{

    class Renderer3D
    {
        public:
            static void Init();
            static void BeginDraw();
            static void EndDraw();
            static void BeginScene(const PerspectiveCamera& camera);
            static void EndScene();
            static void Shutdown();
            // static void SubmitScene(Scene* scene);
            // static void ReleaseScene(Scene* scene);

        public:
            static void DrawModel(uint32_t id, const glm::mat4& transform);
            // static void DrawModel(uint32_t id, float transform);
            // static void DrawModel(Model* model);
            // static void DrawModel(GameObject* object);

        public:
            static void SubmitModel(Model* model);

        private:
            static void BindPipeline(VkCommandBuffer cmd);
            static void UpdateUniformData(UniformBufferObject ubd);
            static void BindDescriptorSets(
                VkCommandBuffer cmd, Mesh& mesh, Pipeline& pipeLine,
                VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                uint32_t firstSet = 0, uint32_t dSetCount = 1,
                uint32_t dynamicOffsetCount = 0,
                uint32_t* dynamicOffsets    = nullptr);
    };
}  // namespace FooGame
