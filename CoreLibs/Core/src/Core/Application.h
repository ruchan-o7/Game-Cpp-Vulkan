#pragma once

#include <mutex>
#include "../Core/Assert.h"
#include "../Core/Window.h"
#include "LayerStack.h"
#include "../Events/ApplicationEvent.h"
#include "../ImGui/ImGuiLayer.h"
int main(int argc, char** argv);

namespace FooGame
{
    class Layer;
    struct ApplicationCommandLineArgs
    {
            int Count   = 0;
            char** Args = nullptr;

            const char* operator[](int index) const
            {
                FOO_ASSERT(index < Count);
                assert(index < Count);
                return Args[index];
            }
    };
    struct ApplicationSpecifications
    {
            String Name = "Foo Application";
            String WorkingDirectory;
            ApplicationCommandLineArgs CommandLineArgs;
    };
    class Application
    {
        public:
            Application(const ApplicationSpecifications& spec);
            virtual ~Application();

            void OnEvent(Event& e);

            void PushLayer(Layer* layer);
            void PushOverlay(Layer* layer);

            Window& GetWindow() { return *m_Window; }

            void Close();

            static Application& Get() { return *s_Instance; }

            const ApplicationSpecifications& GetSpecifications() const { return m_Specs; }

            void SubmitToMainThread(const std::function<void()>& function);
            void ExecuteMainThreadQueue();

        private:
            void Run();
            bool OnWindowClose(WindowCloseEvent& e);
            bool OnWindowResize(WindowResizeEvent& e);

        private:
            ApplicationSpecifications m_Specs;
            Unique<Window> m_Window;
            ImGuiLayer* m_ImGuiLayer = nullptr;
            bool m_Running           = true;
            bool m_Minimized         = false;
            LayerStack m_LayerStack;
            float m_LastFrameTime = 0.0f;

            List<std::function<void()>> m_MainThreadQueue;
            std::mutex m_MainThreadQueueMutex;

        private:
            static Application* s_Instance;
            friend int ::main(int argc, char** argv);
    };
    Application* CreateApplication(ApplicationCommandLineArgs args);
}  // namespace FooGame
