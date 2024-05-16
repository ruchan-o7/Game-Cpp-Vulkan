#pragma once
#include "Core/Core/Base.h"
#include "Core/Graphics/Api.h"
#include "Core/Graphics/Model.h"
#include "Core/Scene/GameObject.h"
#include "Core/Scene/Scene.h"
#include "vulkan/vulkan_core.h"
namespace FooGame
{

    class OrthographicCamera;
    class PerspectiveCamera;
    class Renderer3D
    {
        public:
            static void Init();
            static void BeginDraw();
            static void EndDraw();
            static void BeginScene(const PerspectiveCamera& camera);
            static void EndScene();
            static void Shutdown();
            static void SubmitScene(const Shared<Scene>& scene);
            static void ReleaseScene();

        public:
            // static void DrawModel();
            static void DrawModel(const Shared<GameObject>& object);

        public:
            static void SubmitModel(const Shared<Model>& model);

        private:
            static void BindPipeline(VkCommandBuffer cmd);
            static void UpdateUniformData(UniformBufferObject ubd);
            static void BindDescriptorSets(
                VkCommandBuffer cmd, Mesh& mesh, Pipeline& pipeLine,
                VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                u32 firstSet = 0, u32 dSetCount = 1, u32 dynamicOffsetCount = 0,
                u32* dynamicOffsets = nullptr);
    };
}  // namespace FooGame
