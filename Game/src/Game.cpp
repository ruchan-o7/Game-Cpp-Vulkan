#include "Game.h"
#include <cmath>
#include <iostream>
#include "Core/Core/Base.h"
#include "Core/Core/Renderer2D.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/Event.h"
#include "Core/Input/KeyCodes.h"
#include "GLFW/glfw3.h"
#include <algorithm>
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
        Camera camera{};

        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();
            {
                Renderer2D::ResetStats();
                Renderer2D::BeginScene(camera);
                Renderer2D::DrawQuad({0.f, 0.5f}, {1.f, 2.f},
                                     {1.0f, 0.0f, 1.0f, 1.0f});
                float val  = std::clamp(sin(glfwGetTime()), .2, 1.0);
                auto color = glm::vec3{val * .2, val * .3, val * .5};
                Renderer2D::SetClearColor(color);
                Renderer2D::EndDraw();
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
        Renderer2D::Resize();
        return true;
    }
}  // namespace FooGame
