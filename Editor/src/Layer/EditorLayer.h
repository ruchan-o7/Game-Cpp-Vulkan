#pragma once
#include "Layer.h"
#include <Core.h>
#include "../Core/CommandLineArgs.h"

namespace FooGame
{
    class EditorLayer final : public Layer
    {
        public:
            EditorLayer(const CommandLineArgs& args);

            void OnAttach() override;
            void OnDetach() override;
            void OnRender() override;
            void OnUpdate(float ts) override;
            void OnImGuiRender() override;
            void OnEvent(Event& event) override;

        private:
            PerspectiveCamera m_Camera;
            Camera m_Camera2;
            std::unique_ptr<Scene> m_Scene;
            CommandLineArgs m_Args;

        private:
            bool OnMouseMoved(MouseMovedEvent& event);
            bool OnMousePressed(MouseButtonPressedEvent& event);
            void UpdateCamera(float ts);
            void DrawCameraUI();
    };
}  // namespace FooGame
