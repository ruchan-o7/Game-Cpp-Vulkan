#pragma once
#include <string>
#include <Engine.h>
namespace FooGame
{
    class Layer
    {
        public:
            Layer(const std::string& name = "Layer");
            virtual ~Layer() = default;

            virtual void OnAttach() {}
            virtual void OnDetach() {}
            virtual void OnUpdate(float ts) {}
            virtual void OnImGuiRender() {}
            virtual void OnEvent(Event& e) {}

            const std::string& GetName() const { return m_DebugName; }

        protected:
            std::string m_DebugName;
    };
}  // namespace FooGame