#include "Editor.h"
#include "Layer/EditorLayer.h"

#include <Core.h>
#include <imgui.h>
#include <nlohmann/json.hpp>

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
