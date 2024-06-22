#pragma once
#include <filesystem>
#include "Scene.h"
#include "../Base.h"
#include "json.hpp"
#include <future>
namespace FooGame
{
    class SceneSerializer
    {
            using json = nlohmann::json;

        public:
            SceneSerializer(String scenePath, Scene* scene);
            void DeSerialize();
            void Serialize();

        private:
            void SerializeEntity(Entity& entity, nlohmann::json& sceneEntitiesNode);
            void DeSerializeEntity(const json& entityJson, Scene* scene,
                                   std::vector<std::future<void>>& futures);

        private:
            String m_ScenePathStr;
            Scene* m_pScene;
            std::filesystem::path m_ScenePath;
            std::filesystem::path m_AssetFolder;
            std::filesystem::path m_ModelsFolder;
            std::filesystem::path m_ImagesFolder;
            std::filesystem::path m_MaterialsFolder;
    };
}  // namespace FooGame
