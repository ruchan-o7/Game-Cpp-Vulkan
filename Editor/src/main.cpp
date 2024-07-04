#include "Editor.h"
#include <src/Core/EntryPoint.h>
namespace FooGame
{

    Application* CreateApplication(ApplicationCommandLineArgs args)
    {
        ApplicationSpecifications specs;
        specs.Name            = "Level editor";
        specs.CommandLineArgs = args;
        return new Editor(specs);
    }
}  // namespace FooGame
