#include "SceneSerializer.h"
#include <pch.h>
#include <Log.h>
#include "AssetSerializer.h"
#include "Entity.h"
#include "Asset.h"
#include <stb_image.h>
#include <cstddef>
#include <filesystem>
#include "../Engine/Core/VulkanTexture.h"
#include "../Base.h"
#include "../Core/AssetManager.h"
#include "../Scene/Component.h"
#include "../Scripts/Rotate.h"
#include "../Scripts/ScaleYoink.h"
#include "../Scripts/CameraController.h"
#include "../Core/File.h"
#include "../Config.h"
#include "../Core/Assert.h"
#include "../Scene/ObjectConverters/ModelConverter.h"

namespace FooGame
{
    using json = nlohmann::json;
    class ScriptableEntity;

    SceneSerializer::SceneSerializer(String scenePath, Scene* scene)
        : m_ScenePathStr(scenePath), m_pScene(scene)
    {
        m_ScenePath = File::GetCWD() / std::filesystem::path(m_ScenePathStr);
        File::SetSceneBasePath(m_ScenePath.parent_path());
    }

    void SceneSerializer::SerializeEntity(Entity& entity, json& sceneEntitiesNode)
    {
        if (!entity.HasComponent<IDComponent>())
        {
            u64 id                  = entity.GetComponent<IDComponent>().ID;
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
        if (!entity.HasComponent<ModelRendererComponent>())
        {
            auto mrc    = entity.GetComponent<ModelRendererComponent>();
            u64 modelId = mrc.AssetModelId;
            auto* asset = AssetManager::GetModelAsset(modelId);
            FOO_ASSERT(asset, "Referenced asset is null");
            sceneEntitiesNode["meshComponent"] = json::object({
                {"modelId", modelId},
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
    void SceneSerializer::DeSerializeEntity(const json& entityJson, Scene* scene,
                                            List<std::future<void>>& futures)
    {
        uint64_t uuid = entityJson["id"];
        String tag;
        bool componentExists = false;

        componentExists   = entityJson.contains("tagComponent");
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
                auto& mc        = deserializedEntity.AddComponent<ModelRendererComponent>();
                mc.AssetModelId = meshComponentJson["modelId"].get<u64>();
            }
        }
        componentExists = entityJson.contains("scriptComponent");
        if (componentExists)
        {
            auto scriptComponent = entityJson["scriptComponent"];
            if (!scriptComponent.empty())
            {
                auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
                for (auto& scriptName : scriptComponent.get<List<String>>())
                {
                    auto res = Script::HasScriptExists(scriptName);
                    if (!res)
                    {
                        FOO_CORE_ERROR("Script could not found: {0}", scriptName);
                        FOO_CORE_WARN("Did you forget to REGISTER_SCRIPT({0})", scriptName);
                        continue;
                    }
                    sc.Bind(scriptName);
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
        scene["name"] = m_pScene->m_Name.empty() ? "New Scene" : m_pScene->m_Name;
        scene["id"]   = (u64)m_pScene->m_Id;

        json imageAssetsJson;
        json materialAssetsJson;
        json modelAssetsJson;

        json FModelJsons;
        json FImageJsons;
        json FMatJsons;

        ModelSerializer ms;
        ImageSerializer is;
        MaterialSerializer mats;

        auto CreateAssetItemJson = [](const String& name, u64 id)
        {
            json a;
            a["name"] = name;
            a["id"]   = id;
            return a;
        };
        std::set<u64> materialIdsToSerialize;
        std::set<u64> imageIdsToSerialize;

        auto AddMaterialId = [&](u64 id)
        {
            if (id != DEFAULT_MATERIAL_ID)
            {
                materialIdsToSerialize.insert(id);
            }
        };
        auto AddImageId = [&](u64 id)
        {
            if (id != DEFAULT_TEXTURE_ID)
            {
                imageIdsToSerialize.insert(id);
            }
        };

        /// MESH
        FOO_CORE_TRACE("Asset Models Serializing");
        auto meshRendererComponents = m_pScene->GetAllEntitiesWith<ModelRendererComponent>();
        meshRendererComponents.each(
            [&](ModelRendererComponent& c)
            {
                auto* modelAsset = AssetManager::GetModelAsset(c.AssetModelId);
                if (modelAsset == nullptr)
                {
                    return;
                }
                auto model       = modelAsset->Asset;
                u64 assetId      = modelAsset->Id;
                String assetName = modelAsset->Name;

                FOO_CORE_TRACE("Model asset deserializing:\tId:{0}\t Name:{1}", assetName, assetId);

                json assetItem = CreateAssetItemJson(modelAsset->Name, assetId);
                modelAssetsJson.push_back(assetItem);

                Asset::FModel fmodel = ModelToFModel(*model);
                for (const auto& fmesh : fmodel.Meshes)
                {
                    for (const auto& p : fmesh.Primitives)
                    {
                        AddMaterialId(p.MaterialId);
                    }
                }
                FModelJsons.emplace_back(ms.Serialize(fmodel));
            });
        FOO_CORE_TRACE("Asset Models serialization done!");

        /// MATERIALS
        FOO_CORE_TRACE("Asset Materials serializing");
        for (auto& materialId : materialIdsToSerialize)
        {
            auto* materialAsset = AssetManager::GetMaterialAsset(materialId);
            if (!materialAsset)
            {
                continue;
            }
            u64 id         = materialAsset->Id;
            json assetItem = CreateAssetItemJson(materialAsset->Name, id);
            materialAssetsJson.push_back(assetItem);

            AddImageId(materialAsset->Asset->BaseColorTexture.id);
            AddImageId(materialAsset->Asset->NormalTextureId);
            AddImageId(materialAsset->Asset->RoughnessTextureId);
            AddImageId(materialAsset->Asset->MetallicTextureId);

            FMatJsons.emplace_back(std::move(mats.Serialize(*materialAsset->Asset)));
        }
        FOO_CORE_TRACE("Asset Materials serialization done");

        /// IMAGES
        FOO_CORE_TRACE("Asset Images serializing");
        for (auto& imageId : imageIdsToSerialize)
        {
            auto* imageAsset = AssetManager::GetTextureAsset(imageId);
            if (imageAsset->Id == DEFAULT_TEXTURE_ID || imageId == 0)
            {
                continue;
            }
            auto imageName = imageAsset->Name;
            u64 id         = imageAsset->Id;
            imageAssetsJson.emplace_back(std::move(CreateAssetItemJson(imageName, id)));

            Asset::FImage fimg;
            fimg.Name         = imageName;
            fimg.Size         = imageAsset->Asset->m_Info.Size;
            fimg.Width        = imageAsset->Asset->m_Info.Width;
            fimg.Height       = imageAsset->Asset->m_Info.Height;
            fimg.ChannelCount = imageAsset->Asset->m_Info.ChannelCount;
            if (imageAsset->Asset->m_Info.Format == VK_FORMAT_R8G8B8A8_SRGB)
            {
                fimg.Format = Asset::TextureFormat::RGBA8;
            }
            else
            {
                fimg.Format = Asset::TextureFormat::RGB8;
            }
            FImageJsons.push_back(is.Serialize(fimg));
        }
        FOO_CORE_TRACE("Asset Images serialization done!");

        scene["assets"]["images"]    = imageAssetsJson;
        scene["assets"]["materials"] = materialAssetsJson;
        scene["assets"]["models"]    = modelAssetsJson;
        auto WriteAssetDataToFile =
            [](const json& assets, const String& extension, std::filesystem::path& basePath)
        {
            for (auto& asset : assets)
            {
                auto assetName = basePath / asset["name"];
                File::WriteJsonData(assetName, extension, asset);
            }
        };
        auto imagesPath    = File::GetImagesPath();
        auto materialsPath = File::GetMaterialsPath();
        auto modelsPath    = File::GetModelsPath();
        WriteAssetDataToFile(FImageJsons, FIMAGE_ASSET_EXTENSION, imagesPath);
        WriteAssetDataToFile(FMatJsons, FMATERIAL_ASSET_EXTENSION, materialsPath);
        WriteAssetDataToFile(FModelJsons, FMODEL_ASSET_EXTENSION, modelsPath);
        /// ENTITIES
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

        auto scenefile = File::GetCWD() / std::filesystem::path(m_ScenePathStr);
        File::WriteJsonData(scenefile, ".json", scene);
    }
    void SceneSerializer::DeSerialize()
    {
        if (!std::filesystem::exists(m_ScenePath))
        {
            m_pScene->m_Name = DEFAULT_SCENE_NAME;
            m_pScene->m_Id   = UUID();
            FOO_CORE_WARN("No scene file found, creating new one");
            return;
        }
        json sceneJson;
        String sceneName;
        {
            std::ifstream is(m_ScenePath);
            DEFER(is.close());
            try
            {
                sceneJson = json::parse(is);
            }
            catch (const std::exception& e)
            {
                FOO_CORE_ERROR("Can not load scene {0}", e.what());
                return;
            }
            sceneName = sceneJson["name"].get<String>();
            if (sceneName.empty())
            {
                FOO_CORE_WARN("Scene does not has sceneName. Setting as 'New Scene'");
                sceneName = "New Scene";
            }
        }
        m_pScene->m_Name        = sceneName;
        m_pScene->m_Id          = sceneJson["id"].get<u64>();
        auto assetsJson         = sceneJson["assets"];
        auto imageAssetsJson    = assetsJson["images"];
        auto materialAssetsJson = assetsJson["materials"];
        auto modelAssetsJson    = assetsJson["models"];

        MaterialSerializer mats;
        ImageSerializer ims;
        ModelSerializer ms;

        struct FimageAsset
        {
                Asset::FImage image;
                UUID id;
        };
        struct FModelAsset
        {
                Asset::FModel model;
                UUID id;
        };
        List<FimageAsset> fimages;
        List<FModelAsset> fModels;

        FOO_CORE_TRACE("Scene deserializing ({0})", m_pScene->m_Name);
        auto PrintJsonParseExceptionError = [](const String& name, const std::exception& e)
        {
            FOO_CORE_ERROR("Error occured while parsing asset ({0}), here is error detail: {1}",
                           name, e.what());
        };

        if (!materialAssetsJson.empty())
        {
            for (auto& mAssetJson : materialAssetsJson)
            {
                UUID id          = mAssetJson["id"].get<u64>();
                String assetName = mAssetJson["name"];

                FOO_CORE_TRACE("Material asset deserializing\t ID:{0} \t Name:{1}", (u64)id,
                               assetName);

                std::filesystem::path assetMaterialDataPath = File::GetMaterialsPath() / assetName;

                assetMaterialDataPath.replace_extension(FMATERIAL_ASSET_EXTENSION);
                if (!std::filesystem::exists(assetMaterialDataPath))
                {
                    FOO_CORE_WARN("Material does not exists: {0},\n Searched path: {1}", assetName,
                                  assetMaterialDataPath.string());
                    continue;
                }

                std::ifstream is{assetMaterialDataPath};
                DEFER(is.close());
                json matJ;
                try
                {
                    matJ = json::parse(is);
                }
                catch (const std::exception& e)
                {
                    PrintJsonParseExceptionError(assetName, e);
                    continue;
                }

                Asset::FMaterial fMat = std::move(mats.DeSerialize(matJ));

                auto* pfmat = new Asset::FMaterial(fMat);

                AssetManager::AddMaterial(Shared<Asset::FMaterial>(pfmat), id);
            }
        }
        else
        {
            FOO_CORE_TRACE("No material asset registered");
        }

        if (!modelAssetsJson.empty())
        {
            for (auto& mAssetJson : modelAssetsJson)
            {
                UUID id          = mAssetJson["id"].get<u64>();
                String assetName = mAssetJson["name"];

                FOO_CORE_TRACE("Model asset deserializing\t ID:{0} \t Name:{1}", (u64)id,
                               assetName);

                auto assetModelDataPath = File::GetModelsPath() / assetName;
                assetModelDataPath.replace_extension(FMODEL_ASSET_EXTENSION);

                if (!std::filesystem::exists(assetModelDataPath))
                {
                    FOO_CORE_WARN("Model does not exists: {0},\n Searched path: {1}", assetName,
                                  assetModelDataPath.string());
                    continue;
                }

                std::ifstream is{assetModelDataPath};
                DEFER(is.close());
                json modelAssetJsonData;
                try
                {
                    modelAssetJsonData = json::parse(is);
                }
                catch (const std::exception& e)
                {
                    PrintJsonParseExceptionError(assetName, e);
                    continue;
                }
                auto fM = std::move(ms.DeSerialize(modelAssetJsonData));
                FModelAsset fmodel;
                fmodel.id    = id;
                fmodel.model = std::move(fM);
                fModels.emplace_back(std::move(fmodel));
            }
        }
        else
        {
            FOO_CORE_TRACE("No model asset registered");
        }
        if (!imageAssetsJson.empty())
        {
            for (auto& imj : imageAssetsJson)
            {
                UUID id          = imj["id"].get<u64>();
                String assetName = imj["name"];
                FOO_CORE_TRACE("Image asset deserializing\t ID:{0} \t Name:{1}", (u64)id,
                               assetName);
                std::filesystem::path assetModelDataPath = File::GetImagesPath() / assetName;
                assetModelDataPath.replace_extension(FIMAGE_ASSET_EXTENSION);
                if (!std::filesystem::exists(assetModelDataPath))
                {
                    FOO_CORE_WARN("Image does not exists: {0},\n Searched path: {1}", assetName,
                                  assetModelDataPath.string());
                    continue;
                }
                std::ifstream is{assetModelDataPath};
                DEFER(is.close());
                json fimJ;
                try
                {
                    fimJ = json::parse(is);
                }
                catch (const std::exception& e)
                {
                    PrintJsonParseExceptionError(assetName, e);
                    continue;
                }
                auto fimage = ims.DeSerialize(fimJ);
                FimageAsset asset;
                asset.id    = id;
                asset.image = std::move(fimage);
                fimages.emplace_back(std::move(asset));
            }
        }
        else
        {
            FOO_CORE_TRACE("No image asset registered");
        }
        List<std::future<void>> futures;
#if 0
#define ASYNC(x) futures.emplace_back(std::async(std::launch::async, [&] { x; }))
#else
#define ASYNC(x) x;
#endif
        for (size_t i = 0; i < fimages.size(); i++)
        {
            AssetManager::LoadFIMG(fimages[i].image, fimages[i].id);
        }
        for (size_t i = 0; i < fModels.size(); i++)
        {
            ASYNC(AssetManager::LoadModel(fModels[i].model, fModels[i].id));
        }
#undef ASYNC

        auto entities = sceneJson["entities"];
        if (!entities.empty())
        {
            FOO_CORE_TRACE("Starting deserialize entities");
            for (auto entity : entities)
            {
                DeSerializeEntity(entity, m_pScene, futures);
            }
        }
        else
        {
            FOO_CORE_TRACE("No entity entry found!");
        }
        if (futures.size() > 0)
        {
            FOO_CORE_TRACE("Loading registered assets asyncly");
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
