#include <Core/demo.h>
#include "Game.h"
int main()
{
    FooGame::Game game{};
    game.Run();
    Core::PrintHello();
}
