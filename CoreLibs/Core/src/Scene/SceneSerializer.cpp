#include "SceneSerializer.h"
#include <Log.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <utility>
#include "AssetSerializer.h"
#include "Entity.h"
#include "../Engine/Geometry/Model.h"
#include "../Core/AssetManager.h"
#include "../Scene/Component.h"
#include "../Scripts/Rotate.h"
#include "../Scripts/ScaleYoink.h"
#include "../Scripts/CameraController.h"
#include "../Core/File.h"
#include "Asset.h"
#include "src/Core/GltfLoader.h"
#include "src/Core/ObjLoader.h"
#include "src/Log.h"
#include <stb_image.h>
/// Scene path should be base path
///
/// |- Scene Folder - Scene.json
/// |- Assets Folder - Asset Name Folder - Asset.json
/// |- Assets Folder - Asset Name Folder - Asset.fmodel | Asset.fimg
/// |- Assets Folder - Asset Name Folder - Asset.fmat
///
///

namespace FooGame
{
    using json = nlohmann::json;
    class ScriptableEntity;

    SceneSerializer::SceneSerializer(String scenePath, Scene* scene)
        : m_ScenePathStr(scenePath), m_pScene(scene)
    {
        // auto assets          = File::GetAssetDirectory();
        m_ScenePath        = File::GetCWD() / std::filesystem::path(m_ScenePathStr);
        auto sceneBasePath = m_ScenePath.parent_path();
        m_AssetFolder      = sceneBasePath / "Assets";
        m_ModelsFolder     = m_AssetFolder / "Models";
        m_ImagesFolder     = m_AssetFolder / "Images";
        m_MaterialsFolder  = m_AssetFolder / "Materials";
#define CREATE_IF_NOT_EXISTS(x)                        \
    if (!std::filesystem::exists(x.string()))          \
    {                                                  \
        std::filesystem::create_directory(x.string()); \
    }
        CREATE_IF_NOT_EXISTS(m_AssetFolder);
        CREATE_IF_NOT_EXISTS(m_ModelsFolder);
        CREATE_IF_NOT_EXISTS(m_ImagesFolder);
        CREATE_IF_NOT_EXISTS(m_MaterialsFolder);
#undef CREATE_IF_NOT_EXISTS
    }

    void SceneSerializer::SerializeEntity(Entity& entity, json& sceneEntitiesNode)
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
            auto mesh = entity.GetComponent<MeshRendererComponent>();

            sceneEntitiesNode["meshComponent"] = json::object({
                {"model", mesh.ModelName},
                // {"material", mesh.MaterialName}
            });

            // sceneEntitiesNode["meshComponent"] = json::object({
            //     {"modelPath",    mesh.ModelPath},
            //     { "material", mesh.MaterialName}
            // });
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
    void SceneSerializer::DeSerializeEntity(const json& entityJson, Scene* scene,
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
            auto meshComponentJson = entityJson["meshComponent"];
            if (!meshComponentJson.empty())
            {
                auto& mc     = deserializedEntity.AddComponent<MeshRendererComponent>();
                mc.ModelName = meshComponentJson["model"];
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
    void SceneSerializer::Serialize()
    {
        json scene;
        scene["name"] = m_pScene->m_Name;

        ModelSerializer s;
        std::vector<Asset::FModel> modelAssets;
        std::vector<json> modelAssetsJsons;
        auto comps = m_pScene->GetAllEntitiesWith<MeshRendererComponent>();
        comps.each(
            [&](MeshRendererComponent& c)
            {
                auto model = AssetManager::GetModel(c.ModelName);
                if (!model)
                {
                    return;
                }
                Asset::FModel fmodel;
                fmodel.MeshCount = model->m_Meshes.size();
                fmodel.Name      = model->Name;
                for (auto& m : model->m_Meshes)
                {
                    Asset::FMesh mesh;
                    mesh.Name         = m.Name;
                    mesh.MaterialName = m.M3Name;
                    mesh.IndicesCount = m.m_Indices.size();
                    mesh.VertexCount  = m.m_Vertices.size();
                    mesh.TotalSize =
                        m.m_Vertices.size() * 11 * sizeof(float) + m.m_Indices.size() * sizeof(u32);
                    mesh.Vertices.reserve(m.m_Vertices.size() * 11);
                    for (size_t i = 0; i < m.m_Vertices.size(); i++)
                    {
                        auto& v = m.m_Vertices[i];
                        mesh.Vertices.push_back(v.Position.x);
                        mesh.Vertices.push_back(v.Position.y);
                        mesh.Vertices.push_back(v.Position.z);

                        mesh.Vertices.push_back(v.Normal.x);
                        mesh.Vertices.push_back(v.Normal.y);
                        mesh.Vertices.push_back(v.Normal.z);

                        mesh.Vertices.push_back(v.Color.x);
                        mesh.Vertices.push_back(v.Color.y);
                        mesh.Vertices.push_back(v.Color.z);

                        mesh.Vertices.push_back(v.TexCoord.x);
                        mesh.Vertices.push_back(v.TexCoord.y);
                    }
                    mesh.Indices = m.m_Indices;
                    fmodel.Meshes.emplace_back(std::move(mesh));
                }
                modelAssetsJsons.emplace_back(std::move(s.Serialize(fmodel)));
            });
        for (auto& modelAssetsJson : modelAssetsJsons)
        {
            auto modelAssetFileName = m_ModelsFolder / modelAssetsJson["name"];
            modelAssetFileName.replace_extension(".fmodel");
            std::ofstream o{modelAssetFileName};
            DEFER(o.close());
            o << std::setw(4) << modelAssetsJson << std::endl;
        }
        for (auto& modJ : modelAssetsJsons)
        {
            scene["assets"]["models"].push_back(modJ["name"]);
        }

        auto& materials = AssetManager::GetAllMaterials();
#define PARSE_FACTOR(x, y) \
    x[0] = y[0];           \
    x[1] = y[1];           \
    x[2] = y[2];           \
    x[3] = y[3];

        std::vector<json> materialJsons;
        MaterialSerializer ms;
        for (auto& [name, mat] : materials)
        {
            Asset::FMaterial fmat;
            fmat.Name                  = name;
            fmat.BaseColorTexture.Name = mat.PbrMat.BaseColorTextureName;
            PARSE_FACTOR(fmat.BaseColorTexture.factor, mat.PbrMat.BaseColorFactor);

            fmat.MetallicTextureName = mat.PbrMat.MetallicRoughnessTextureName;
            fmat.MetallicFactor      = mat.PbrMat.MetallicFactor;

            fmat.RoughnessTextureName = mat.PbrMat.MetallicRoughnessTextureName;
            fmat.RoughnessFactor      = mat.PbrMat.RoughnessFactor;

            fmat.EmissiveTexture.factor[0] = 1.0f;  // TODO FOR NOW

            fmat.alphaMode   = Asset::AlphaMode::Opaque;  // TODO
            fmat.DoubleSided = false;
            materialJsons.emplace_back(std::move(ms.Serialize(fmat)));
        }
        for (auto& materialAssetJson : materialJsons)
        {
            auto materialAssetFileName = m_MaterialsFolder / materialAssetJson["name"];
            materialAssetFileName.replace_extension(".fmat");
            std::ofstream o{materialAssetFileName};
            DEFER(o.close());
            o << std::setw(4) << materialAssetJson << std::endl;
        }
        for (auto& matJ : materialJsons)
        {
            scene["assets"]["materials"].push_back(matJ["name"]);
        }
#undef PARSE_FACTOR

        std::vector<json> imagesJsons;
        ImageSerializer is;
        auto& images = AssetManager::GetAllImages();
        for (auto& [name, img] : images)
        {
            if (name == "Default Texture")
            {
                continue;
            }
            Asset::FImage fimg;
            fimg.Name         = name;
            fimg.Size         = img->m_Info.Size;
            fimg.Width        = img->m_Info.Width;
            fimg.Height       = img->m_Info.Height;
            fimg.ChannelCount = img->m_Info.ChannelCount;
            if (img->m_Info.Format == VK_FORMAT_R8G8B8A8_SRGB)
            {
                fimg.Format = Asset::TextureFormat::RGBA8;
            }
            else
            {
                fimg.Format = Asset::TextureFormat::RGB8;
            }
            AssetManager::GetTextureFromGPU(name, fimg.Data, fimg.Size);
            imagesJsons.emplace_back(std::move(is.Serialize(fimg)));
        }

        for (auto& imageAssetJson : imagesJsons)
        {
            auto materialAssetFileName = m_ImagesFolder / imageAssetJson["name"];
            materialAssetFileName.replace_extension(".fimg");
            std::ofstream o{materialAssetFileName};
            DEFER(o.close());
            o << std::setw(4) << imageAssetJson << std::endl;
        }
        for (auto& imgJ : imagesJsons)
        {
            scene["assets"]["images"].push_back(imgJ["name"]);
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

        // std::string str = scene.dump();
        std::ofstream o{m_ScenePath};
        o << std::setw(4) << scene << std::endl;
        o.close();
    }
    void SceneSerializer::DeSerialize()
    {
        if (!std::filesystem::exists(m_ScenePath))
        {
            m_pScene->m_Name = "New scene";
            return;
        }
        std::ifstream is(m_ScenePath);
        Defer defer{[&] { is.close(); }};
        json sceneJson;
        try
        {
            sceneJson = json::parse(is);
        }
        catch (const std::exception& e)
        {
            FOO_CORE_ERROR("Can not load scene {0}", e.what());
            return;
        }
        if (sceneJson["name"].empty())
        {
            FOO_CORE_ERROR("Scene does not has name");
            return;
        }

        m_pScene->m_Name = sceneJson["name"];
        FOO_CORE_TRACE("Scene: {0} deserializing", m_pScene->m_Name);
        auto materials = sceneJson["assets"]["materials"];
        if (!materials.empty())
        {
            for (auto& m : materials.get<std::vector<String>>())
            {
                std::filesystem::path matPath = m_MaterialsFolder / m;
                matPath.replace_extension(".fmat");
                if (std::filesystem::exists(matPath))
                {
                    std::ifstream is{matPath};
                    Defer defer{[&] { is.close(); }};
                    json matJ = json::parse(is);
                    MaterialSerializer ms;
                    auto fMat = std::move(ms.DeSerialize(matJ));

                    Material mat{};
                    mat.Name                 = fMat.Name;
                    mat.fromGlb              = false;
                    auto& pbr                = mat.PbrMat;
                    pbr.BaseColorTextureName = fMat.BaseColorTexture.Name;
                    pbr.BaseColorFactor[0]   = fMat.BaseColorTexture.factor[0];
                    pbr.BaseColorFactor[1]   = fMat.BaseColorTexture.factor[1];
                    pbr.BaseColorFactor[2]   = fMat.BaseColorTexture.factor[2];
                    pbr.BaseColorFactor[3]   = fMat.BaseColorTexture.factor[3];

                    pbr.MetallicFactor               = fMat.MetallicFactor;
                    pbr.MetallicRoughnessTextureName = fMat.MetallicTextureName;
                    pbr.RoughnessFactor              = fMat.RoughnessFactor;

                    AssetManager::AddMaterial(mat);
                }
                else
                {
                    FOO_CORE_WARN("Material does not exists: {0},\n Searched path: {1}", m,
                                  matPath.string());
                }
            }
        }
        auto models = sceneJson["assets"]["models"];
        std::vector<Asset::FModel> fModels;
        for (auto& mJ : models.get<std::vector<std::string>>())
        {
            std::filesystem::path modelPath = m_ModelsFolder / mJ;
            modelPath.replace_extension(".fmodel");
            if (std::filesystem::exists(modelPath))
            {
                std::ifstream is{modelPath};
                json fModelJson = json::parse(is, nullptr, false);

                Asset::FModel model;
                model.Name      = fModelJson["name"];
                model.MeshCount = fModelJson["meshCount"];
                for (auto& m : fModelJson["meshes"])
                {
                    Asset::FMesh mesh;
                    mesh.Name            = m["name"];
                    mesh.MaterialName    = m["materialName"];
                    mesh.VertexCount     = m["vertexCount"];
                    mesh.IndicesCount    = m["indicesCount"];
                    mesh.TotalSize       = m["totalSize"];
                    List<float> vertices = m["vertices"];
                    mesh.Vertices        = std::move(vertices);
                    List<u32> indices    = m["indices"];
                    mesh.Indices         = std::move(indices);
                    model.Meshes.emplace_back(std::move(mesh));
                }
                fModels.emplace_back(std::move(model));
            }
            else
            {
                FOO_CORE_WARN("Model does not exists: {0},\n Searched path: {1}", mJ,
                              modelPath.string());
            }
        }
        std::vector<std::future<void>> futures;

#if 1
#define ASYNC(x) futures.emplace_back(std::async(std::launch::async, [=] { x; }))
#else
#define ASYNC(x) x;
#endif
        for (auto& m : fModels)
        {
            ASYNC(AssetManager::LoadModel(m));
        }
#undef ASYNC

        auto entities = sceneJson["entities"];
        if (!entities.empty())
        {
            for (auto entity : entities)
            {
                DeSerializeEntity(entity, m_pScene, futures);
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
