#pragma once
#include "Core/Graphics/Model.h"
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
            static void Flush();
            static void Shutdown();

        public:
            static void DrawModel();
            static void DrawModel(Model& model);

        public:
            static void SubmitModel(const Shared<Model>& model);

        private:
            static void BindPipeline(VkCommandBuffer cmd);
    };
}  // namespace FooGame
