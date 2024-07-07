#include "Editor.h"
#include <Core.h>
#include <nlohmann/json.hpp>
#include "Layer/EditorLayer.h"
#include <Log.h>
#include <imgui.h>
namespace FooGame
{
    Editor::Editor(const ApplicationSpecifications& spec) : Application(spec)
    {
        Init();
        PushLayer(new EditorLayer(spec.CommandLineArgs));
        SetMenubarCallback(
            [&]()
            {
                if (ImGui::BeginMenu("File"))
                {
                    DEFER(ImGui::EndMenu());

                    if (ImGui::MenuItem("Foo"))
                    {
                    }
                }
            });
    }
    void Editor::Init()
    {
        FOO_EDITOR_INFO("Editor instantiating");
    }

    Editor::~Editor()
    {
    }
}  // namespace FooGame
