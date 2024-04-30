#pragma once
namespace FooGame
{
    class Renderer
    {
        public:
            static void Init();
            static void Shutdown();
            static void Resize();
            static void DrawFrame();
            static void BeginDraw();
            static void EndDraw();
    };

}  // namespace FooGame
