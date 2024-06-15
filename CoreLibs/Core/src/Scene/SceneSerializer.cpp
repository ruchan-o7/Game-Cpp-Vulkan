#include "SceneSerializer.h"
#include <Log.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <json.hpp>
#include <vector>
#include "Entity.h"
#include "../Core/AssetManager.h"
#include "../Scene/Component.h"
#include "../Engine/Geometry/Material.h"
#include "../Scripts/Rotate.h"
#include "entt/entity/fwd.hpp"
#include "src/Log.h"
#include "src/Scripts/ScaleYoink.h"
namespace FooGame
{
    using json = nlohmann::json;
    class ScriptableEntity;

    SceneSerializer::SceneSerializer(Scene* scene) : m_pScene(scene)
    {
    }
    void SceneSerializer::DeSerialize(const std::string& path)
    {
        json scene;
        scene["name"]  = m_pScene->m_Name;
        auto materials = AssetManager::AllMaterials();
        for (auto& [name, mat] : materials)
        {
            scene["materials"].push_back({
                {  "name",      mat.Name},
                {"albedo", mat.AlbedoMap}
            });
        }
        scene["entities"] = json::array();
        auto view         = m_pScene->m_Registry.view<entt::entity>().each();
        for (auto [e] : view)
        {
            Entity ent{e, m_pScene};

            if (!ent)
            {
                continue;
            }
            json entityJson = json::object();
            if (!ent.HasComponent<IDComponent>())
            {
                uint64_t id      = ent.GetComponent<IDComponent>().ID;
                entityJson["id"] = id;
            }
            if (!ent.HasComponent<TagComponent>())
            {
                auto tag                   = ent.GetComponent<TagComponent>().Tag;
                entityJson["tagComponent"] = {"tag", tag};
            }

            if (!ent.HasComponent<TransformComponent>())
            {
                auto transform = ent.GetComponent<TransformComponent>();

                entityJson["transformComponent"] = json::object({
                    {"translation",
                     {transform.Translation.x, transform.Translation.y, transform.Translation.z}},
                    {   "rotation",
                     {transform.Rotation.x, transform.Rotation.y, transform.Rotation.z}         },
                    {      "scale",    {transform.Scale.x, transform.Scale.y, transform.Scale.z}}
                });
            }
            if (!ent.HasComponent<MeshRendererComponent>())
            {
                auto mesh                   = ent.GetComponent<MeshRendererComponent>();
                entityJson["meshComponent"] = json::object({
                    {"modelPath",    mesh.ModelPath},
                    { "material", mesh.MaterialName}
                });
            }
            if (!ent.HasComponent<ScriptComponent>())
            {
                auto scripts                  = ent.GetComponent<ScriptComponent>();
                entityJson["scriptComponent"] = json::array();
                for (auto [name, script] : scripts.Scripts)
                {
                    entityJson["scriptComponent"].push_back(name);
                }
            }
            scene["entities"].push_back(entityJson);
        }

        std::string str = scene.dump();
        std::ofstream o{path};
        o << std::setw(4) << scene << std::endl;
        o.close();
    }
    void SceneSerializer::Serialize(const std::string& path)
    {
        auto cwd       = std::filesystem::current_path();
        auto assetPath = cwd / "Assets";

        auto sceneBasePath = cwd / path;

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
        auto materials = sceneJson["materials"];
        if (!materials.empty())
        {
            for (auto& m : materials)
            {
                Material2 mat{};
                mat.Name      = m["name"];
                mat.AlbedoMap = std::filesystem::path(m["albedo"]).filename().string();
                AssetManager::AddMaterial(mat);
                auto texPath = assetPath / std::filesystem::path(m["albedo"]);

                AssetManager::LoadTexture(texPath.string(), texPath.filename().string());
            }
        }
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
                assert(!transformComponent.empty());
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
                    auto& mc        = deserializedEntity.AddComponent<MeshRendererComponent>();
                    auto modelPath  = std::filesystem::path(meshComponent["modelPath"]);
                    mc.ModelPath    = modelPath.string();
                    mc.MaterialName = meshComponent["material"];

                    auto modelExtension = modelPath.extension().string();
                    if (modelExtension == ".obj")
                    {
                        auto p = (assetPath / modelPath);
                        AssetManager::LoadObjModel(p.string(), p.filename().string());
                        mc.ModelName = p.filename().string();
                    }
                    else if (modelExtension == ".glb")
                    {
                        auto p = (assetPath / modelPath);
                        AssetManager::LoadGLTFModel(p.string(), p.filename().string(), true);
                        mc.ModelName = p.filename().string();
                    }
                    else if (modelExtension == ".gltf")
                    {
                        auto p = (assetPath / modelPath);
                        AssetManager::LoadGLTFModel(p.string(), p.filename().string(), false);
                        mc.ModelName = p.filename().string();
                    }
                }
                auto scriptComponent = entity["scriptComponent"];
                if (!scriptComponent.empty())
                {
                    auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
                    for (auto& sname : scriptComponent)
                    {
                        if (sname == "Rotate")
                        {
                            sc.Bind<RotateScript>("RotateScript");
                        }
                        else if (sname == "ScaleYoink")
                        {
                            sc.Bind<ScaleYoink>("ScaleYoink");
                        }
                        else
                        {
                            FOO_ENGINE_ERROR("Script with name: {0} could not found");
                        }
                    }
                }
            }
        }
    }
}  // namespace FooGame
