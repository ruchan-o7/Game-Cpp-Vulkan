#pragma once
#include "Scene.h"
namespace FooGame
{
    class SceneSerializer
    {
        public:
            SceneSerializer(Scene* scene);
            void Serialize(const std::string& path);
            void DeSerialize(const std::string& path);

        private:
            Scene* m_pScene;
    };
}  // namespace FooGame
