#pragma once
#include "EditorScene.h"
#include "Layer.h"
#include "../Core/CommandLineArgs.h"

namespace FooGame
{
    class EditorLayer final : public Layer
    {
        public:
            EditorLayer(const CommandLineArgs& args);

            void OnAttach() override;
            void OnDetach() override;
            void OnUpdate(float ts) override;
            void OnImGuiRender() override;
            void OnEvent(Event& event) override;

        private:
            PerspectiveCamera m_Camera;
            Camera m_Camera2;
            std::unique_ptr<EditorScene> m_EditorScene;
            CommandLineArgs m_Args;
            bool OnMouseMoved(MouseMovedEvent& event);
            bool OnMousePressed(MouseButtonPressedEvent& event);

        private:
            void UpdateCamera(float ts);
            void DrawMeshUI();
            void DrawCameraUI();
    };
}  // namespace FooGame
