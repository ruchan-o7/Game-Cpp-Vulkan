#include "Game.h"
#include <src/Core/EntryPoint.h>
namespace FooGame
{
    Application* CreateApplication(ApplicationCommandLineArgs args)
    {
        ApplicationSpecifications specs;
        specs.Name            = "Level editor";
        specs.CommandLineArgs = args;
        return new Game(specs);
    }
}  // namespace FooGame
