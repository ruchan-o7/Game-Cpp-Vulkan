#pragma once
#include <Core.h>
#include "src/Core/Application.h"

namespace FooGame
{
    class SceneHierarchyPanel;
    class EditorLayer final : public Layer
    {
        public:
            EditorLayer(const ApplicationCommandLineArgs& args);

            void OnAttach() override;
            void OnDetach() override;
            void OnUpdate(float ts) override;
            void OnImGuiRender() override;
            void OnEvent(Event& event) override;

        private:
            // PerspectiveCamera m_Camera;
            Camera m_Camera2;
            std::unique_ptr<Scene> m_Scene;
            ApplicationCommandLineArgs m_Args;
            SceneHierarchyPanel* m_HierarchyPanel;

        private:
            bool OnMouseMoved(MouseMovedEvent& event);
            bool OnMousePressed(MouseButtonPressedEvent& event);
    };
}  // namespace FooGame
