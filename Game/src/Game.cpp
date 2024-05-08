#include "Game.h"
#include <pch.h>
#include "Core/Core/Base.h"
#include "Core/Core/Engine.h"
#include "Core/Core/PerspectiveCamera.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/Event.h"
#include "Core/Input/KeyCodes.h"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include <Core/Graphics/Camera.h>
#include <cstdio>
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

    double deltaTime__     = 0;
    double lastFrameTime__ = 0;
    void Game::Run()
    {
        m_Engine = Engine::Create(m_Window->GetWindowHandle());
        m_Camera.RecalculateViewMatrix();
        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();

            double currentTime = glfwGetTime();
            deltaTime__        = currentTime - lastFrameTime__;
            lastFrameTime__    = currentTime;
            m_Engine->Start();
            m_Engine->BeginScene(m_Camera);
            static float rotation  = 0.0f;
            rotation              += deltaTime__ + 2.f;
            for (u32 i = 0; i < m_BenchmarkAmount; i++)
            {
                for (u32 j = 0; j < m_BenchmarkAmount; j++)
                {
                    glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
                    glm::vec2 size{0.1f, 0.1f};
                    static float offset = 1.0f;
                    glm::vec2 pos{(i * 0.1f) - offset, (j * 0.1f) - offset};
                    m_Engine->DrawQuad(pos, size, color);
                    // m_Engine->DrawRotatedQuad(pos, size, rotation, color);
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
            char title[255];
            sprintf(
                title,
                "Benchmark amount %i, total Quad: %i, total Vertex amount: %i",
                m_BenchmarkAmount, m_BenchmarkAmount * m_BenchmarkAmount,
                m_BenchmarkAmount * m_BenchmarkAmount * 4);
            m_Window->SetWindowTitle(title);
        }
        if (key.GetKeyCode() == KeyCode::Down)
        {
            m_BenchmarkAmount--;

            char title[255];
            sprintf(
                title,
                "Benchmark amount %i, total Quad: %i, total Vertex amount: %i",
                m_BenchmarkAmount, m_BenchmarkAmount * m_BenchmarkAmount,
                m_BenchmarkAmount * m_BenchmarkAmount * 4);
            m_Window->SetWindowTitle(title);
        }

        if (key.GetKeyCode() == KeyCode::A)
        {
            m_Camera.GoLeft();
        }

        if (key.GetKeyCode() == KeyCode::D)
        {
            m_Camera.GoRight();
        }
        if (key.GetKeyCode() == KeyCode::W)
        {
            m_Camera.GoForward();
        }

        if (key.GetKeyCode() == KeyCode::S)
        {
            m_Camera.GoBackward();
        }
        if (key.GetKeyCode() == KeyCode::Q)
        {
            m_Camera.TurnRight();
        }
        if (key.GetKeyCode() == KeyCode::E)
        {
            m_Camera.TurnLeft();
        }
        if (key.GetKeyCode() == KeyCode::Space)
        {
            m_Camera.GoUp();
        }
        if (key.GetKeyCode() == KeyCode::LeftControl)
        {
            m_Camera.GoDown();
        }
        if (key.GetKeyCode() == KeyCode::RightShift)
        {
            m_Camera.m_Zoom += 10.0f;
        }
        if (key.GetKeyCode() == KeyCode::LeftShift)
        {
            m_Camera.m_Zoom -= 10.0f;
        }

        m_Camera.RecalculateViewMatrix();
        return false;
    }
    bool Game::OnWindowResized(WindowResizeEvent& event)
    {
        m_Camera.SetAspect((float)event.GetWidth() / (float)event.GetHeight());
        return m_Engine->OnWindowResized(event);
    }
}  // namespace FooGame
