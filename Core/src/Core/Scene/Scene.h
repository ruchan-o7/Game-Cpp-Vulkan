#pragma once
#include <unordered_map>
#include "Core/Core/Base.h"
#include "Core/Core/OrthographicCamera.h"
#include "Core/Core/PerspectiveCamera.h"
#include "Core/Events/Event.h"
#include "Core/Graphics/Model.h"
#include "Core/Scene/GameObject.h"
namespace FooGame
{
    class Scene
    {
            DELETE_COPY(Scene);

        public:
            Scene() = default;
            virtual void OnAttach(){};
            virtual void OnDetach(){};
            virtual void OnUpdate(float deltaTime){};
            virtual void OnRender(){};
            virtual void OnUI(){};
            virtual void OnEvent(Event& event){};

        public:
    };
    class SampleScene final : public Scene
    {
            DELETE_COPY(SampleScene)
        public:
            SampleScene();
            void OnAttach() override;
            void OnUpdate(float deltaTime) override;
            void OnUI() override;
            void OnRender() override;
            std::unordered_map<String, Shared<GameObject>> Objects;

        private:
            PerspectiveCamera m_Camera;
            OrthographicCamera m_Ortho{0, 1, 0, -1};
    };
}  // namespace FooGame
