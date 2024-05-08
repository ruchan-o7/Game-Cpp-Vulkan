#pragma once
#include "Base.h"

namespace FooGame
{
    class OrthographicCamera
    {
        public:
            OrthographicCamera(glm::vec3 pos);

        private:
            glm::vec3 m_Position;
            glm::vec3 m_Front;
            glm::vec3 m_Up;
    };
}  // namespace FooGame
