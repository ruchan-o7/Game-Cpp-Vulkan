#pragma once
#include <Engine.h>
#include "Core/Layer.h"
#include "Core/CommandLineArgs.h"
#include "Core/LayerStack.h"
namespace FooGame
{
    class WindowsWindow;
    class Editor
    {
        public:
            Editor(CommandLineArgs args);
            ~Editor();
            void Run();
            void PushLayer(Layer* layer);

        private:
            void Init();
            void OnEvent(Event& event);
            bool OnKeyEvent(KeyPressedEvent& key);
            bool OnWindowResized(WindowResizeEvent& event);
            bool OnMouseMoved(MouseMovedEvent& event);
            bool OnMouseScroll(MouseScrolledEvent& event);
            bool OnMousePressed(MouseButtonPressedEvent& event);
            bool OnMouseRelease(MouseButtonReleasedEvent& event);

        private:
            WindowsWindow* m_Window;
            LayerStack* m_LayerStack;
    };
}  // namespace FooGame
