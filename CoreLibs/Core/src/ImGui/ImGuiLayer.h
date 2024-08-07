#pragma once
#include <functional>
#include "../Core/Layer.h"
#include "../Events/Event.h"
namespace FooGame
{
    class ImGuiLayer : public Layer
    {
        public:
            ImGuiLayer();
            ~ImGuiLayer() = default;

            virtual void OnAttach() override;
            virtual void OnDetach() override;
            virtual void OnEvent(Event& e) override;

            void Begin(std::function<void()>* callback = nullptr);
            void End();

            void BlockEvents(bool block) { m_BlockEvents = block; }

        private:
            bool m_BlockEvents = true;
    };

}  // namespace FooGame
