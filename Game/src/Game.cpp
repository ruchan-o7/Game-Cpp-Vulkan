#include "Game.h"
#include <pch.h>
#include <Engine.h>
#include "Core/Core/Base.h"
#include "Core/Scene/Scene.h"
#include "imgui.h"
#include "src/Engine/Backend.h"
#define GLM_ENABLE_EXPERIMENTAL
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
        Backend::Init(*m_Window);
        Renderer2D::Init();
        Renderer3D::Init();
        m_Scenes.emplace_back(new Scene());
    }
    static void DrawQuads(int amount, const Shared<Texture2D> texture,
                          float tiling, glm::vec4 tint)
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
    void Game::Run()
    {
        double lastTime = 0;

        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();
            double currentTime = m_Window->GetTime();
            m_DeltaTime        = currentTime - lastTime;
            lastTime           = currentTime;

            for (auto& l : m_Scenes)
            {
                l->OnUpdate(currentTime);
            }

            Backend::BeginDrawing();
            for (auto& l : m_Scenes)
            {
                l->RenderScene3D(&m_Camera);
                // l->OnRender();
                l->IMGUI();
            }
            Backend::EndDrawing();
        }
    }

    void Game::Shutdown()
    {
        Renderer2D::Shutdown();
        Renderer3D::Shutdown();
        Backend::Shutdown();
        m_Scenes.clear();
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

        return false;
    }
    bool Game::OnWindowResized(WindowResizeEvent& event)
    {
        return Backend::OnWindowResized(event);
    }
    bool Game::OnMouseMoved(MouseMovedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(event.GetX(), event.GetY());
        return true;
    }
    bool Game::OnMouseScroll(MouseScrolledEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseWheelEvent(event.GetXOffset(), event.GetYOffset());
        return true;
    }
    bool Game::OnMousePressed(MouseButtonPressedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(event.GetMouseButton(), true);
        if (event.GetMouseButton() == Mouse::Button1)
        {
            m_Window->SetCursorCenter();
        }
        return true;
    }
    bool Game::OnMouseRelease(MouseButtonReleasedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(event.GetMouseButton(), false);
        return true;
    }
}  // namespace FooGame
