#include "Game.h"
#include <pch.h>
#include "Core/Core/Base.h"
#include "Core/Core/Engine.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/Event.h"
#include "Core/Input/KeyCodes.h"
#include "glm/fwd.hpp"
#include <thread>
#include <Core/Graphics/Camera.h>
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

        m_RenderThread = std::thread(
            [&]()
            {
                m_Engine = Engine::Create(m_Window->GetWindowHandle());
                m_Engine->RunLoop();
            });
        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();
        }
        std::cout << "Render Engine Closing" << std::endl;
        m_Engine->Close();
        std::cout << "Render Engine Closed !" << std::endl;
        m_RenderThread.join();
        std::cout << "Application closing" << std::endl;
    }

    void Game::Shutdown()
    {
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
        return false;
    }
    bool Game::OnWindowResized(WindowResizeEvent& event)
    {
        return true;
    }
}  // namespace FooGame
