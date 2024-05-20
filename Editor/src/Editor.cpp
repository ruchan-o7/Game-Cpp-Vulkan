#include "Editor.h"
#include <Engine.h>
#include <iostream>
#include <Scene/Scene.h>
#include "Core/Base.h"
#include "imgui.h"
#include "src/Engine/Backend.h"
#include "src/Engine/Renderer3D.h"
#include "src/Events/Event.h"
#include "Core/EditorLayer.h"
#include "src/Input/KeyCodes.h"
#include <nlohmann/json.hpp>
#include "Core/EditorLayer.h"
namespace FooGame
{
    Editor::Editor(CommandLineArgs args) : m_Window(nullptr)
    {
        std::cout << "Editor instantiating" << std::endl;
        Init();
        PushLayer(new EditorLayer(args));
    }
    void Editor::Init()
    {
        WindowProperties properties{};
        properties.Title  = "Level editor";
        properties.Width  = 1600;
        properties.Height = 900;
        m_Window          = new WindowsWindow(properties);
        m_Window->SetOnEventFunction(
            [this](auto&&... args) -> decltype(auto)
            { return Editor::OnEvent(std::forward<decltype(args)>(args)...); });
        Backend::Init(*m_Window);
        Renderer3D::Init();
    }
    void Editor::Run()
    {
        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();
            Backend::BeginDrawing();

            for (auto& l : m_LayerStack)
            {
                l->OnUpdate(0.2f);
            }
            for (auto& l : m_LayerStack)
            {
                l->OnImGuiRender();
            }

            Backend::EndDrawing();
        }
        Backend::WaitIdle();
    }
    void Editor::OnEvent(Event& e)
    {
        EventDispatcher dispatcher{e};
        dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(Editor::OnKeyEvent));
        dispatcher.Dispatch<WindowResizeEvent>(
            BIND_EVENT_FN(Editor::OnWindowResized));
        dispatcher.Dispatch<MouseMovedEvent>(
            BIND_EVENT_FN(Editor::OnMouseMoved));
        dispatcher.Dispatch<MouseScrolledEvent>(
            BIND_EVENT_FN(Editor::OnMouseScroll));
        dispatcher.Dispatch<MouseButtonPressedEvent>(
            BIND_EVENT_FN(Editor::OnMousePressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(
            BIND_EVENT_FN(Editor::OnMouseRelease));
        for (auto& l : m_LayerStack)
        {
            l->OnEvent(e);
        }
    }
    bool Editor::OnKeyEvent(KeyPressedEvent& key)
    {
        if (key.GetKeyCode() == KeyCode::Escape)
        {
            std::cout << "Closing..." << std::endl;
            m_Window->Close();
            return true;
        }

        return false;
    }
    void Editor::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }
    bool Editor::OnWindowResized(WindowResizeEvent& event)
    {
        return Backend::OnWindowResized(event);
    }
    bool Editor::OnMouseMoved(MouseMovedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(event.GetX(), event.GetY());
        return true;
    }
    bool Editor::OnMouseScroll(MouseScrolledEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseWheelEvent(event.GetXOffset(), event.GetYOffset());
        return true;
    }
    bool Editor::OnMousePressed(MouseButtonPressedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(event.GetMouseButton(), true);
        if (event.GetMouseButton() == Mouse::Button1)
        {
            m_Window->SetCursorCenter();
        }
        return true;
    }
    bool Editor::OnMouseRelease(MouseButtonReleasedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(event.GetMouseButton(), false);
        return true;
    }

    Editor::~Editor()
    {
        for (auto* l : m_LayerStack)
        {
            m_LayerStack.PopOverlay(l);
        }
        Renderer3D::Shutdown();
        Backend::Shutdown();
        delete m_Window;
    }
}  // namespace FooGame
