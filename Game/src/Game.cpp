#include "Game.h"
#include <pch.h>
#include "Core/Core/Base.h"
#include "Core/Core/Engine.h"
#include "Core/Core/PerspectiveCamera.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/Event.h"
#include "Core/Events/MouseMovedEvent.h"
#include "Core/Input/KeyCodes.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
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
            ImGui::Begin("Benchmark");
            ImGui::SliderInt("Amount", &m_BenchmarkAmount, 10, 1000);
            ImGui::End();
            if (ImGui::Button("Test", {200, 200}))
            {
                std::cout << "Clicked" << std::endl;
            }
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
        m_Engine->Close();
    }

    void Game::Shutdown()
    {
        delete m_Engine;
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
        dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(Game::OnMouseMoved));
        dispatcher.Dispatch<MouseScrolledEvent>(
            BIND_EVENT_FN(Game::OnMouseScroll));
        dispatcher.Dispatch<MouseButtonPressedEvent>(
            BIND_EVENT_FN(Game::OnMousePressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(
            BIND_EVENT_FN(Game::OnMouseRelease));
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
            String title{std::move(StrFormat(
                "Benchmark amount %i, total Quad: %i, total Vertex amount: %i",
                m_BenchmarkAmount, m_BenchmarkAmount * m_BenchmarkAmount,
                m_BenchmarkAmount * m_BenchmarkAmount * 4))};
            m_Window->SetWindowTitle(title.c_str());
            title.clear();
        }
        if (key.GetKeyCode() == KeyCode::Down)
        {
            m_BenchmarkAmount--;

            String title{std::move(StrFormat(
                "Benchmark amount %i, total Quad: %i, total Vertex amount: %i",
                m_BenchmarkAmount, m_BenchmarkAmount * m_BenchmarkAmount,
                m_BenchmarkAmount * m_BenchmarkAmount * 4))};
            m_Window->SetWindowTitle(title.c_str());
            title.clear();
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
        if (key.GetKeyCode() == KeyCode::Space)
        {
            m_Camera.GoUp();
        }
        if (key.GetKeyCode() == KeyCode::LeftControl)
        {
            m_Camera.GoDown();
        }

        m_Camera.RecalculateViewMatrix();
        return false;
    }
    bool Game::OnWindowResized(WindowResizeEvent& event)
    {
        m_Camera.SetAspect((float)event.GetWidth() / (float)event.GetHeight());
        return m_Engine->OnWindowResized(event);
    }
    bool Game::OnMouseMoved(MouseMovedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(event.GetX(), event.GetY());
        if (m_SecondMouse)
        {
            auto newPos = m_Window->GetCursorPos();
            m_Window->SetCursorCenter();
            auto centerPos = m_Window->GetCursorPos();
            int xOffset    = centerPos.first - newPos.first;
            int yOffset    = centerPos.second - newPos.second;

            float sens  = 0.1f;
            xOffset    *= sens;
            yOffset    *= sens;
            m_Camera.Look({xOffset, yOffset});
            m_Camera.RecalculateViewMatrix();
        }

        return true;
    }
    bool Game::OnMouseScroll(MouseScrolledEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseWheelEvent(event.GetXOffset(), event.GetYOffset());
        m_Camera.Zoom(event.GetYOffset() * -1.f);
        m_Camera.RecalculateViewMatrix();
        return true;
    }
    bool Game::OnMousePressed(MouseButtonPressedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(event.GetMouseButton(), true);
        if (event.GetMouseButton() == Mouse::Button1)
        {
            m_Window->SetCursorCenter();
            m_SecondMouse = true;
        }
        return true;
    }
    bool Game::OnMouseRelease(MouseButtonReleasedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(event.GetMouseButton(), false);
        if (event.GetMouseButton() == Mouse::Button1)
        {
            m_SecondMouse = false;
        }
        return true;
    }
}  // namespace FooGame
