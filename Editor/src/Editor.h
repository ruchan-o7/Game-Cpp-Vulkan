#pragma once

#include <Core.h>
#include <src/Core/Application.h>
namespace FooGame
{
    class Window;
    class Editor : public Application
    {
        public:
            Editor(const ApplicationSpecifications& spec);
            ~Editor();

        private:
            void Init();

        private:
            bool m_ShouldRender = true;
    };
}  // namespace FooGame
