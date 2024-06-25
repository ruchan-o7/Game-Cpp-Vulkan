#include "Application.h"
#include <mutex>
#include "../Engine/Engine/Backend.h"
#include "../Engine/Engine/Renderer3D.h"
#include "../ImGui/ImGuiLayer.h"
#include "../Core/AssetManager.h"
#include "../Core/Time.h"
#include "../Events/Event.h"
#include <imgui.h>
namespace FooGame
{
    Application* Application::s_Instance = nullptr;
    Application::Application(const ApplicationSpecifications& spec) : m_Specs(spec)
    {
        FOO_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        if (!m_Specs.WorkingDirectory.empty())
        {
            std::filesystem::current_path(m_Specs.WorkingDirectory);
        }

        WindowProperties props;
        props.Title = m_Specs.Name;

        m_Window = CreateUnique<Window>(props);
        m_Window->SetOnEventFunction(BIND_EVENT_FN(Application::OnEvent));

        Backend::Init(*m_Window);
        Renderer3D::Init(Backend::GetRenderDevice());
        AssetManager::Init();

        m_ImGuiLayer = new ImGuiLayer;
        PushLayer(m_ImGuiLayer);
    }
    Application::~Application()
    {
        AssetManager::DeInit();
        Renderer3D::Shutdown();
        Backend::Shutdown();
    }
    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }
    void Application::Close()
    {
        m_Running = false;
    }

    void Application::SubmitToMainThread(const std::function<void()>& f)
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
        m_MainThreadQueue.emplace_back(f);
    }
    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher{e};
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
        for (auto it = m_LayerStack.begin(); it != m_LayerStack.end(); ++it)
        {
            if (e.Handled)
            {
                break;
            }
            (*it)->OnEvent(e);
        }
    }
    void Application::Run()
    {
        while (m_Running)
        {
            Time::UpdateCurrentTime();
            auto time       = Time::CurrentTime();
            float ts        = time - m_LastFrameTime;
            m_LastFrameTime = time;
            ExecuteMainThreadQueue();
            if (!m_Minimized)
            {
                m_ImGuiLayer->Begin();
                for (Layer* l : m_LayerStack)
                {
                    l->OnUpdate(ts);
                }
                auto stats = Renderer3D::GetStats();

                Renderer3D::EndDraw();
                ImGui::Begin("3d scene stats");
                ImGui::Text("Draw calls %i", stats.DrawCall);
                ImGui::Text("Vertex count %llu", stats.VertexCount);
                ImGui::Text("Index count %llu", stats.IndexCount);
                ImGui::End();

                for (Layer* l : m_LayerStack)
                {
                    l->OnImGuiRender();
                }
                m_ImGuiLayer->End();
            }
            m_Window->PollEvents();
            Backend::SwapBuffers();
        }
    }
    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }
    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }
        m_Minimized = false;
        // TODO: Notify Backend window resized
        return false;
    }
    void Application::ExecuteMainThreadQueue()
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
        for (auto& f : m_MainThreadQueue)
        {
            f();
        }
        m_MainThreadQueue.clear();
    }
}  // namespace FooGame
