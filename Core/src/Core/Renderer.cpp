#include "Renderer.h"
#include "Render/VulkanContext.h"
namespace FooGame
{
    // consider using stack alloc
    Context* Renderer::s_Context = nullptr;
    void Renderer::Init()
    {
        s_Context = new Context;
        s_Context->Init();
    }

    void Renderer::DrawFrame()
    {
        s_Context->DrawFrame();
    }
    void Renderer::Shutdown()
    {
        // TODO:
    }

    void Renderer::Resize()
    {
        s_Context->ResizeSwapchain();
    }
    void Renderer::BeginDraw()
    {
        s_Context->BeginDraw();
    }

    VkDevice Renderer::GetDevice()
    {
        return s_Context->GetDevice();
    }
    void Renderer::EndDraw()
    {
        s_Context->EndDraw();
    }

}  // namespace FooGame
