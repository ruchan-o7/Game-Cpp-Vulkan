#include "Game.h"
#include <pch.h>
#include "Core/Core/Base.h"
#include "Core/Core/Engine.h"
#include "Core/Core/OrthographicCamera.h"
#include "Core/Core/PerspectiveCamera.h"
#include "Core/Core/Window.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/Event.h"
#include "Core/Events/MouseMovedEvent.h"
#include "Core/Graphics/Renderer2D.h"
#include "Core/Graphics/Renderer3D.h"
#include "Core/Input/KeyCodes.h"
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
        m_Tex = CreateShared<Texture2D>();
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
        Engine::Init(*m_Window);
        m_Camera.RecalculateViewMatrix();

        OrthographicCamera m_OrthoCamera{-1.0f, 1.0f, -1.0f, 1.0f};
        // m_OrthoCamera.MoveTo({2.0f, 2.0f, 2.0f});

        m_Camera.RecalculateViewMatrix();
        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();
            Engine::BeginDrawing();

            double currentTime = m_Window->GetTime();
            {
                {
                    Renderer2D::BeginDrawing();
                    {
                        Renderer2D::BeginScene(m_OrthoCamera);
                        // DrawQuads(m_BenchmarkAmount, m_Tex, m_Tilin,
                        //           m_Tint);
                        Renderer2D::DrawQuad({0.0f, 0.0f}, {1.0f, 1.0f}, m_Tex,
                                             m_Tilin, m_Tint);
                        Renderer2D::EndScene();
                    }
                    Renderer2D::EndDrawing();
                }
                {
                    Renderer3D::BeginDraw();
                    {
                        Renderer3D::BeginScene(m_Camera);
                        Renderer3D::DrawModel();
                        Renderer3D::EndScene();
                    }
                    Renderer2D::EndDrawing();
                }

                {
                    ImGui::Begin("Benchmark");
                    ImGui::SliderInt("Amount", &m_BenchmarkAmount, 10, 1000);
                    auto posisitons = m_Camera.GetPosition();
                    float pos[3] = {posisitons.x, posisitons.y, posisitons.z};
                    ImGui::SliderFloat3("Camera pos", pos, -10.0f, 10.0f);
                    m_Camera.SetPosition({pos[0], pos[1], pos[2]});

                    auto posisitonsO = m_OrthoCamera.GetPosition();
                    float posO[3]    = {posisitonsO.x, posisitonsO.y,
                                        posisitonsO.z};
                    ImGui::SliderFloat3("Camera pos Ortho", posO, -10.0f,
                                        10.0f);
                    m_OrthoCamera.SetPosition({posO[0], posO[1], posO[2]});
                    float values[4] = {
                        m_OrthoCamera.m_Left,
                        m_OrthoCamera.m_Right,
                        m_OrthoCamera.m_Bottom,
                        m_OrthoCamera.m_Top,
                    };
                    ImGui::SliderFloat4("Camera val", values, -10.0f, 10.0f);
                    m_OrthoCamera.SetProj(values[0], values[1], values[2],
                                          values[3]);
                    ImGui::SliderFloat("Tiling", &m_Tilin, 0.0f, 30.0f);
                    float tint[4] = {m_Tint.x, m_Tint.y, m_Tint.z, m_Tint.w};
                    ImGui::ColorEdit4("Tint", tint);
                    m_Tint.x = tint[0];
                    m_Tint.y = tint[1];
                    m_Tint.z = tint[2];
                    m_Tint.w = tint[3];
                    ImGui::End();
                }
            }
            Engine::EndDrawing();
        }
    }

    void Game::Shutdown()
    {
        Engine::Shutdown();
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
        return Engine::OnWindowResized(event);
    }
    bool Game::OnMouseMoved(MouseMovedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(event.GetX(), event.GetY());
        if (m_SecondMouse)
        {
            double offsetX = m_Window->GetCursorPosX();
            double offsetY = m_Window->GetCursorPosY();
            m_Window->SetCursorCenter();
            auto centerPosX = m_Window->GetCursorPosX();
            auto centerPosY = m_Window->GetCursorPosY();
            int xOffset     = centerPosX - offsetX;
            int yOffset     = centerPosY - offsetY;

            float sens  = 0.1f;
            xOffset    *= sens;
            yOffset    *= sens;
            m_Camera.Look(xOffset, yOffset);
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
