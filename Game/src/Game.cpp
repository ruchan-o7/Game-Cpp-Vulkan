#include "Game.h"
#include <pch.h>
#include "Core/Core/Base.h"
#include "Core/Core/Renderer2D.h"
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
                m_Engine.Init(m_Window->GetWindowHandle());
                m_Engine.RunLoop();
            });
        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();
            {
                // float val = std::clamp(sin(glfwGetTime()), .2, 1.0);
                // camera.MoveTo(glm::vec3{val, 0.0f, 0.0f});
                // Renderer2D::ResetStats();
                // Renderer2D::BeginScene(camera);
                // Renderer2D::DrawQuad({0.f, 0.5f}, {1.f, 2.f},
                //                      {1.0f, 0.0f, 1.0f, 1.0f});
                // auto color = glm::vec3{val * .2, val * .3, val * .5};
                // Renderer2D::SetClearColor(color);
                // Renderer2D::EndDraw();
            }
        }
        m_Engine.Close();
        m_RenderThread.join();
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
