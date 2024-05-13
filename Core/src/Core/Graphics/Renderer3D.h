#pragma once
#include "Core/Graphics/Model.h"
namespace FooGame
{

    class OrthographicCamera;
    class PerspectiveCamera;
    class Renderer3D
    {
        public:
            static void Init();
            // static void BeginDraw();
            // static void EndDraw();
            // static void BeginScene(const PerspectiveCamera& camera);
            // static void EndScene();
            // static void Flush();
        public:
            static void SubmitModel(const Shared<Model>& model);
    };
}  // namespace FooGame
