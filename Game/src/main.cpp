#include <Core/demo.h>
#include "Application.h"
#include <exception>
#include <iostream>
int main()
{
    try
    {
        FooGame::Game game{};
        game.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what();
    }
    Core::PrintHello();
}
