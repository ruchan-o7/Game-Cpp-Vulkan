#include "Editor.h"
#include <Engine.h>
#include "src/Engine/Backend.h"
#include "src/Window/Window.h"
namespace FooGame
{
    Editor::Editor()
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
        Backend::Init(*m_Window);
    }
    void Editor::Run()
    {
        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();
        }
    }

    Editor::~Editor()
    {
        delete m_Window;
    }
}  // namespace FooGame
