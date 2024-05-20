#pragma once
#include "EditorScene.h"
#include "Layer.h"
#include "CommandLineArgs.h"
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
            std::unique_ptr<EditorScene> m_EditorScene;
            CommandLineArgs m_Args;

        private:
            void UpdateCamera();
    };
}  // namespace FooGame
