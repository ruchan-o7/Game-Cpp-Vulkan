#include "SceneSerializer.h"
#include <Log.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <json.hpp>
#include "Entity.h"
#include "../Core/AssetManager.h"
#include "../Scene/Component.h"
#include "../Scripts/Rotate.h"
#include "../Scripts/ScaleYoink.h"
#include "../Scripts/CameraController.h"
#include "../Engine/Geometry/Model.h"
#include "../Core/File.h"
namespace FooGame
{
    using json = nlohmann::json;
    class ScriptableEntity;

    SceneSerializer::SceneSerializer(Scene* scene) : m_pScene(scene)
    {
    }
    static void SerializeEntity(Entity& entity, json& sceneEntitiesNode)
    {
        if (!entity.HasComponent<IDComponent>())
        {
            uint64_t id             = entity.GetComponent<IDComponent>().ID;
            sceneEntitiesNode["id"] = id;
        }
        if (!entity.HasComponent<TagComponent>())
        {
            auto tag                                 = entity.GetComponent<TagComponent>().Tag;
            sceneEntitiesNode["tagComponent"]["tag"] = tag;
        }

        if (!entity.HasComponent<TransformComponent>())
        {
            auto transform = entity.GetComponent<TransformComponent>();

            sceneEntitiesNode["transformComponent"] = json::object({
                {"translation",
                 {transform.Translation.x, transform.Translation.y, transform.Translation.z}      },
                {   "rotation", {transform.Rotation.x, transform.Rotation.y, transform.Rotation.z}},
                {      "scale",          {transform.Scale.x, transform.Scale.y, transform.Scale.z}}
            });
        }
        if (!entity.HasComponent<MeshRendererComponent>())
        {
            auto mesh                          = entity.GetComponent<MeshRendererComponent>();
            sceneEntitiesNode["meshComponent"] = json::object({
                {"modelPath",    mesh.ModelPath},
                { "material", mesh.MaterialName}
            });
        }
        if (!entity.HasComponent<ScriptComponent>())
        {
            auto scripts = entity.GetComponent<ScriptComponent>();
            if (!scripts.Scripts.empty())
            {
                sceneEntitiesNode["scriptComponent"] = json::array();
                for (auto [name, script] : scripts.Scripts)
                {
                    sceneEntitiesNode["scriptComponent"].push_back(name);
                }
            }
        }
        if (!entity.HasComponent<CameraComponent>())
        {
            auto camC                            = entity.GetComponent<CameraComponent>();
            sceneEntitiesNode["cameraComponent"] = json::object({
                {         "primary",          camC.Primary},
                {"fixedAspectRatio", camC.FixedAspectRatio}
            });
        }
    }
    static void DeSerializeEntity(const json& entityJson, Scene* scene,
                                  const std::filesystem::path& assetPath,
                                  std::vector<std::future<void>>& futures)
    {
        uint64_t uuid = entityJson["id"];
        std::string tag;
        bool componentExists = false;

        componentExists = entityJson.contains("tagComponent");
        // TODO: Add assertion
        auto tagComponent = entityJson["tagComponent"];
        if (!tagComponent.empty())
        {
            tag = tagComponent["tag"];
        }
        FOO_ENGINE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, tag);
        Entity deserializedEntity = scene->CreateEntityWithUUID(uuid, tag);

        componentExists         = entityJson.contains("transformComponent");
        auto transformComponent = entityJson["transformComponent"];
        assert(!transformComponent.empty());
        if (!transformComponent.empty())
        {
            auto& tc    = deserializedEntity.GetComponent<TransformComponent>();
            auto toVec3 = [](auto comp) { return glm::vec3{comp[0], comp[1], comp[2]}; };

            tc.Translation = toVec3(transformComponent["translation"]);
            tc.Scale       = toVec3(transformComponent["scale"]);
            tc.Rotation    = toVec3(transformComponent["rotation"]);
        }
        componentExists = entityJson.contains("meshComponent");
        if (componentExists)
        {
#define ASYNC(x) futures.emplace_back(std::async(std::launch::async, [=] { x; }))
            auto meshComponent = entityJson["meshComponent"];
            if (!meshComponent.empty())
            {
                auto& mc = deserializedEntity.AddComponent<MeshRendererComponent>();
                auto modelPath =
                    std::filesystem::path(meshComponent["modelPath"].get<std::string>());
                mc.ModelPath    = modelPath.string();
                mc.MaterialName = meshComponent["material"];

                auto modelExtension = modelPath.extension().string();
                if (modelExtension == ".obj")
                {
                    auto p       = (assetPath / modelPath);
                    mc.ModelName = File::ExtractFileName(p);

                    ASYNC(AssetManager::LoadObjModel(p.string(), mc.ModelName, mc.MaterialName));
                    // std::shared_ptr<Model> modelPtr = AssetManager::GetModel(mc.ModelName);

                    // modelPtr->m_Meshes[0].M3Name = mc.MaterialName;
                }
                else if (modelExtension == ".glb")
                {
                    auto p       = (assetPath / modelPath);
                    mc.ModelName = File::ExtractFileName(p);
                    ASYNC(AssetManager::LoadGLTFModel(p.string(), mc.ModelName, true));
                }
                else if (modelExtension == ".gltf")
                {
                    auto p       = (assetPath / modelPath);
                    auto pStr    = p.string();
                    mc.ModelName = File::ExtractFileName(p);
                    ASYNC(AssetManager::LoadGLTFModel(pStr, mc.ModelName, false));
                    // futures.emplace_back(
                    //     std::async(std::launch::async, [=]
                    //                { AssetManager::LoadGLTFModel(pStr, mc.ModelName, false); }));
                    // AssetManager::LoadGLTFModel(pStr, mc.ModelName, false);
                }
            }
        }
        componentExists = entityJson.contains("scriptComponent");
        if (componentExists)
        {
            auto scriptComponent = entityJson["scriptComponent"];
            if (!scriptComponent.empty())
            {
                auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
                for (std::string sname : scriptComponent)
                {
                    if (sname == "RotateScript")
                    {
                        sc.Bind<RotateScript>("RotateScript");
                    }
                    else if (sname == "ScaleYoink")
                    {
                        sc.Bind<ScaleYoink>("ScaleYoink");
                    }
                    else if (sname == "CameraController")
                    {
                        sc.Bind<CameraController>("CameraController");
                    }
                    else
                    {
                        FOO_ENGINE_ERROR("Script with name: {0} could not found", sname);
                    }
                }
            }
        }
        componentExists = entityJson.contains("cameraComponent");
        if (componentExists)
        {
            auto cameraComponent = entityJson["cameraComponent"];
            if (!cameraComponent.empty())
            {
                auto& sc            = deserializedEntity.AddComponent<CameraComponent>();
                sc.Primary          = cameraComponent["primary"];
                sc.FixedAspectRatio = cameraComponent["fixedAspectRatio"];
            }
        }
    }
    void SceneSerializer::Serialize(const std::string& path)
    {
        auto cwd       = std::filesystem::current_path();
        auto assets    = cwd.append("Assets");
        auto scenePath = assets / path;

        json scene;
        scene["name"]  = m_pScene->m_Name;
        auto materials = AssetManager::GetAllMaterials();
        for (auto& [name, mat] : materials)
        {
            json materialJ                       = json::object();
            materialJ["name"]                    = mat.Name;
            materialJ["normalTexture"]           = json::object();
            materialJ["normalTexture"]["name"]   = mat.NormalTexture.Name;
            materialJ["fromGlb"]                 = mat.fromGlb;
            json pbrJ                            = json::object();
            pbrJ["baseColorTexturePath"]         = mat.PbrMat.BaseColorTexturePath;
            pbrJ["baseColorTextureName"]         = mat.PbrMat.BaseColorTextureName;
            pbrJ["metallicRoughnessTextureName"] = mat.PbrMat.MetallicRoughnessTextureName;
            pbrJ["metallicRoughnessTexturePath"] = mat.PbrMat.MetallicRoughnessTexturePath;
            pbrJ["metallicFactor"]               = mat.PbrMat.MetallicFactor;
            pbrJ["roughnessFactor"]              = mat.PbrMat.RoughnessFactor;
            pbrJ["baseColorFactor"]              = mat.PbrMat.BaseColorFactor;
            materialJ["pbrMetallicRougness"]     = pbrJ;

            scene["materials"].push_back(materialJ);
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
            SerializeEntity(ent, entityJson);
            scene["entities"].push_back(entityJson);
        }

        std::string str = scene.dump();
        std::ofstream o{scenePath};
        o << std::setw(4) << scene << std::endl;
        o.close();
    }
    void SceneSerializer::DeSerialize(const std::string& path)
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
                Material mat{};
                mat.Name               = m["name"];
                mat.fromGlb            = m["fromGlb"];
                mat.NormalTexture.Name = m["normalTexture"]["name"];
                auto& pbrM             = m["pbrMetallicRougness"];

                mat.PbrMat.BaseColorTextureName = pbrM["baseColorTextureName"];
                mat.PbrMat.BaseColorTexturePath = pbrM["baseColorTexturePath"];

                mat.PbrMat.MetallicRoughnessTextureName = pbrM["metallicRoughnessTextureName"];
                mat.PbrMat.MetallicRoughnessTexturePath = pbrM["metallicRoughnessTexturePath"];

                mat.PbrMat.BaseColorFactor[0] = pbrM["baseColorFactor"][0];
                mat.PbrMat.BaseColorFactor[1] = pbrM["baseColorFactor"][1];
                mat.PbrMat.BaseColorFactor[2] = pbrM["baseColorFactor"][2];
                mat.PbrMat.BaseColorFactor[3] = pbrM["baseColorFactor"][3];

                mat.PbrMat.MetallicFactor  = pbrM["metallicFactor"];
                mat.PbrMat.RoughnessFactor = pbrM["roughnessFactor"];

                AssetManager::AddMaterial(mat);
                if (!mat.fromGlb)
                {
                    if (!mat.PbrMat.BaseColorTextureName.empty())
                    {
                        auto assetPathStr = (assetPath / mat.PbrMat.BaseColorTexturePath).string();
                        AssetManager::LoadTexture(assetPathStr, mat.PbrMat.BaseColorTextureName);
                    }
                    if (!mat.PbrMat.MetallicRoughnessTextureName.empty())
                    {
                        auto assetPathStr =
                            (assetPath / mat.PbrMat.MetallicRoughnessTexturePath).string();

                        AssetManager::LoadTexture(assetPathStr,
                                                  mat.PbrMat.MetallicRoughnessTextureName);
                    }
                }
            }
        }
        auto entities = sceneJson["entities"];
        std::vector<std::future<void>> futures;
        if (!entities.empty())
        {
            for (auto entity : entities)
            {
                // futures.push_back(
                // std::async(std::launch::async, DeSerializeEntity, entity, m_pScene, assetPath));
                DeSerializeEntity(entity, m_pScene, assetPath, futures);
            }
        }
        for (auto& f : futures)
        {
            if (f.valid())
            {
                f.wait();
            }
        }
    }
}  // namespace FooGame
