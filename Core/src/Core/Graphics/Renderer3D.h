// #pragma once
// #include "../Core/Base.h"
// #include "../Scene/Scene.h"
// #include <Engine.h>
// namespace FooGame
// {
//
//     class Renderer3D
//     {
//         public:
//             static void Init();
//             static void BeginDraw();
//             static void EndDraw();
//             static void BeginScene(const PerspectiveCamera& camera);
//             static void EndScene();
//             static void Shutdown();
//             static void SubmitScene(Scene* scene);
//             static void ReleaseScene(Scene* scene);
//
//         public:
//             static void DrawModel(u32 id, const glm::mat4& transform);
//             // static void DrawModel(u32 id, float transform);
//             // static void DrawModel(Model* model);
//             // static void DrawModel(GameObject* object);
//
//         public:
//             static void SubmitModel(Model* model);
//
//         private:
//             static void BindPipeline(VkCommandBuffer cmd);
//             static void UpdateUniformData(UniformBufferObject ubd);
//             static void BindDescriptorSets(
//                 VkCommandBuffer cmd, Mesh& mesh, Pipeline& pipeLine,
//                 VkPipelineBindPoint bindPoint =
//                 VK_PIPELINE_BIND_POINT_GRAPHICS, u32 firstSet = 0, u32
//                 dSetCount = 1, u32 dynamicOffsetCount = 0, u32*
//                 dynamicOffsets = nullptr);
//     };
// }  // namespace FooGame
