#include "Editor.h"
#include <Engine.h>
namespace FooGame
{
    Editor::Editor() : m_Window(nullptr)
    {
        Init();
    }
    void Editor::Init()
    {
        WindowProperties properties{};
        properties.Title  = "Level editor";
        properties.Width  = 1600;
        properties.Height = 900;
        m_Window          = new WindowsWindow(properties);
        m_Window->SetOnEventFunction(
            [this](auto&&... args) -> decltype(auto)
            { return Editor::OnEvent(std::forward<decltype(args)>(args)...); });
        Backend::Init(*m_Window);
        Renderer3D::Init();
    }
    void Editor::Run()
    {
        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();
            Backend::BeginDrawing();
            Backend::EndDrawing();
        }
    }
    void Editor::OnEvent(Event& e)
    {
    }

    Editor::~Editor()
    {
        Renderer3D::Shutdown();
        Backend::Shutdown();
        delete m_Window;
    }
}  // namespace FooGame
