#include "SceneSerializer.h"
#include <Log.h>
#include <filesystem>
#include <fstream>
#include <json.hpp>
#include "Entity.h"
#include "src/Scene/Component.h"
namespace FooGame
{
    using json = nlohmann::json;

    SceneSerializer::SceneSerializer(Scene* scene) : m_pScene(scene)
    {
    }
    void SceneSerializer::Serialize(const std::string& path)
    {
        auto cwd = std::filesystem::current_path();
        cwd.append(path);
        auto sceneBasePath = std::filesystem::canonical(cwd);

        std::ifstream is(sceneBasePath);
        json sceneJson;
        try
        {
            sceneJson = json::parse(is);
        }
        catch (const std::exception& e)
        {
            is.close();
            FOO_CORE_ERROR("Can not load scene {0}", e.what());
            return;
        }
        is.close();
        if (sceneJson["name"].empty())
        {
            FOO_CORE_ERROR("Scene does not has name");
            return;
        }

        m_pScene->m_Name = sceneJson["name"];
        FOO_CORE_TRACE("Scene: {0} deserializing", m_pScene->m_Name);
        auto entities = sceneJson["entities"];
        if (!entities.empty())
        {
            for (auto entity : entities)
            {
                uint64_t uuid = entity["id"];
                std::string name;

                auto tagComponent = entity["tagComponent"];
                if (!tagComponent.empty())
                {
                    name = tagComponent["tag"];
                }
                FOO_ENGINE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);
                Entity deserializedEntity = m_pScene->CreateEntityWithUUID(uuid, name);

                auto transformComponent = entity["transformComponent"];
                if (!transformComponent.empty())
                {
                    auto& tc    = deserializedEntity.GetComponent<TransformComponent>();
                    auto toVec3 = [](auto comp) { return glm::vec3{comp[0], comp[1], comp[2]}; };

                    tc.Translation = toVec3(transformComponent["translation"]);
                    tc.Scale       = toVec3(transformComponent["scale"]);
                    tc.Rotation    = toVec3(transformComponent["rotation"]);
                }
                auto meshComponent = entity["meshComponent"];
                if (!meshComponent.empty())
                {
                    auto& mc = deserializedEntity.AddComponent<MeshRendererComponent>();
                }
            }
        }
    }
}  // namespace FooGame
