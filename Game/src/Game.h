#pragma once
#include <Core.h>
#include <src/Core/Application.h>
#include <vector>
namespace FooGame
{
    class Game : public Application
    {
        public:
            Game(const ApplicationSpecifications& spec);
            ~Game();

        private:
            std::vector<Scene*> m_Scenes;
            PerspectiveCamera m_Camera;

        private:
            float m_DeltaTime = 0.01f;

        private:
            void Init();
    };

}  // namespace FooGame
