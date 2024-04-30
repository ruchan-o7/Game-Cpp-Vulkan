#include "Renderer.h"
#include "Render/VulkanContext.h"
namespace FooGame
{
    void Renderer::Init()
    {
        Context::Init();
    }

    void Renderer::DrawFrame()
    {
        Context::DrawFrame();
    }
    void Renderer::Shutdown()
    {
        // TODO:
    }

    void Renderer::Resize()
    {
        Context::ResizeSwapchain();
    }
    void Renderer::BeginDraw()
    {
        Context::BeginDraw();
    }
    void Renderer::EndDraw()
    {
        Context::EndDraw();
    }

}  // namespace FooGame
