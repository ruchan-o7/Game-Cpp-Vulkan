#include "Game.h"
#include <Core.h>
#include <Log.h>
#define GLM_ENABLE_EXPERIMENTAL
namespace FooGame
{
    Game::Game(const ApplicationSpecifications& spec) : Application(spec)
    {
        Init();
    }
    void Game::Init()
    {
        m_Scenes.emplace_back(new Scene());
    }
    static void DrawQuads(int amount, const Shared<Texture2D> texture, float tiling, glm::vec4 tint)
    {
        for (u32 i = 0; i < amount; i++)
        {
            for (u32 j = 0; j < amount; j++)
            {
                glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
                glm::vec2 size{0.1f, 0.1f};
                static float offset = 1.0f;
                glm::vec2 pos{(i * 0.1f) - offset, (j * 0.1f) - offset};
                Renderer2D::DrawQuad(pos, size, texture, tiling, tint);
            }
        }
    }

    Game::~Game()
    {
        m_Scenes.clear();
    }

}  // namespace FooGame
