#include "Game.h"
#include <pch.h>
#include "Core/Core/Base.h"
#include "Core/Core/Engine.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/Event.h"
#include "Core/Input/KeyCodes.h"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include <cmath>
#include <Core/Graphics/Camera.h>
#include <vcruntime_new_debug.h>
namespace FooGame
{
    Game::Game()
    {
        Init();
    }
    void Game::Init()
    {
        m_Window = new WindowsWindow();
        m_Window->SetOnEventFunction(BIND_EVENT_FN(Game::OnEvent));
    }

    void Game::Run()
    {
        Camera camera{glm::mat4(1.0f)};

        // m_RenderThread = std::thread(
        //     [&]()
        //     {
        m_Engine = Engine::Create(m_Window->GetWindowHandle());
        // m_Engine->RunLoop();
        //     });
        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();

            m_Engine->Start();
            m_Engine->BeginScene();
            for (u32 i = 0; i < m_BenchmarkAmount; i++)
            {
                for (u32 j = 0; j < m_BenchmarkAmount; j++)
                {
                    glm::vec2 pos{(i * 0.01f) - 0.5f, (j * 0.01f) - 0.5f};
                    m_Engine->DrawQuad(pos, {0.01f, 0.01f},
                                       {1.0f, 1.0f, 1.0f, 1.0f});
                }
            }
            m_Engine->EndScene();
            m_Engine->End();
        }
        std::cout << "Render Engine Closing" << std::endl;
        std::cout << "Application closing" << std::endl;
    }

    void Game::Shutdown()
    {
        delete m_Engine;
        std::cout << "Render Engine Closed !" << std::endl;
        delete m_Window;
    }
    Game::~Game()
    {
        Shutdown();
    }
    void Game::OnEvent(Event& e)
    {
        EventDispatcher dispatcher{e};
        dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(Game::OnKeyEvent));
        dispatcher.Dispatch<WindowResizeEvent>(
            BIND_EVENT_FN(Game::OnWindowResized));
    }

    bool Game::OnKeyEvent(KeyPressedEvent& key)
    {
        if (key.GetKeyCode() == KeyCode::Escape)
        {
            std::cout << "Closing..." << std::endl;
            m_Window->Close();
            return true;
        }
        if (key.GetKeyCode() == KeyCode::Up)
        {
            m_BenchmarkAmount++;
        }
        if (key.GetKeyCode() == KeyCode::Down)
        {
            m_BenchmarkAmount--;
        }
        return false;
    }
    bool Game::OnWindowResized(WindowResizeEvent& event)
    {
        return m_Engine->OnWindowResized(event);
    }
}  // namespace FooGame
