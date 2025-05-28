#include <Core.h>
#include "Game.h"

#define GLM_ENABLE_EXPERIMENTAL
namespace FooGame
{
    Game::Game(const ApplicationSpecifications& spec) : Application(spec)
    {
        // Init();
    }
    void Game::Init()
    {
        // m_Scenes.emplace_back(new Scene());
    }
    Game::~Game()
    {
        m_Scenes.clear();
    }

}  // namespace FooGame
