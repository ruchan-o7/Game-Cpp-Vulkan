#include "Editor.h"
#include <Engine.h>
#include <iostream>
#include "Core/LayerStack.h"
#include "Core/EditorLayer.h"
#include <nlohmann/json.hpp>
#include <Log.h>
namespace FooGame
{
    Editor::Editor(CommandLineArgs args) : m_Window(nullptr)
    {
        Log::Init(AppType::Editor);
        FOO_EDITOR_INFO("Argv {0}", *args.argv);

        Init();
        PushLayer(new EditorLayer(args));
    }
    void Editor::Init()
    {
        FOO_EDITOR_INFO("Editor instantiating");
        WindowProperties properties{};
        properties.Title  = "Level editor";
        properties.Width  = 1600;
        properties.Height = 900;
        m_Window          = new Window(properties);
        m_Window->SetOnEventFunction(
            [this](auto&&... args) -> decltype(auto)
            { return Editor::OnEvent(std::forward<decltype(args)>(args)...); });
        Backend::Init(*m_Window);
        Renderer3D::Init();
        m_LayerStack = new LayerStack;
    }
    void Editor::Run()
    {
        while (!m_Window->ShouldClose())
        {
            m_Window->PollEvents();
            if (!m_ShouldRender)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            Backend::BeginDrawing();

            for (auto& l : *m_LayerStack)
            {
                l->OnUpdate(0.2f);
            }
            for (auto& l : *m_LayerStack)
            {
                l->OnImGuiRender();
            }
            ImGui::Begin("Statistics");
            {
                auto stats = Renderer3D::GetStats();
                auto& io   = ImGui::GetIO();
                ImGui::Text("Draw call :%i", stats.DrawCall);
                ImGui::Text("Vertex count :%llu", stats.VertexCount);
                ImGui::Text("Index count :%llu", stats.IndexCount);
                ImGui::Text("FPS :%f", io.Framerate);
            }
            ImGui::End();
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
        for (auto& l : *m_LayerStack)
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
        m_LayerStack->PushLayer(layer);
        layer->OnAttach();
    }
    bool Editor::OnWindowResized(WindowResizeEvent& event)
    {
        m_ShouldRender = true;
        if (event.GetWidth() == 0 || event.GetHeight() == 0)
        {
            m_ShouldRender = false;
        }

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
        delete m_LayerStack;
        Renderer3D::Shutdown();
        Backend::Shutdown();
        delete m_Window;
    }
}  // namespace FooGame
