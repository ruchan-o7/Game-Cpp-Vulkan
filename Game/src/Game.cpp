#include "Game.h"
#include <iostream>
#include "Core/Core/Base.h"
#include "Core/Core/Renderer.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/Event.h"
#include "Core/Input/KeyCodes.h"
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
        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();
            {
                Renderer::BeginDraw();
                Renderer::DrawFrame();
                Renderer::EndDraw();
            }
        }
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
        Renderer::Resize();
        return true;
    }
}  // namespace FooGame
