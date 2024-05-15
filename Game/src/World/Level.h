#pragma once
#include "Core/Core/Base.h"
#include "Core/Core/PerspectiveCamera.h"
#include "Core/Events/Event.h"
#include "Core/Graphics/Model.h"
namespace FooGame
{
    class Level
    {
            DELETE_COPY(Level)
        public:
            Level() = default;
            virtual void OnAttach(){};
            virtual void OnDetach(){};
            virtual void OnUpdate(float deltaTime){};
            virtual void OnRender(){};
            virtual void OnUI(){};
            virtual void OnEvent(Event& event){};
    };
    class SampleLevel final : public Level
    {
            DELETE_COPY(SampleLevel)
        public:
            SampleLevel();
            void OnAttach() override;
            void OnUpdate(float deltaTime) override;
            void OnUI() override;
            void OnRender() override;

        private:
            PerspectiveCamera m_Camera;
            Shared<Model> m_Model;
    };
}  // namespace FooGame
