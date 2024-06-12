#include <Engine.h>
#include "Scene.h"
#include "../Scene/Entity.h"
#include "../Scene/ScriptableEntity.h"
#include <imgui.h>
#include "../Scene/Component.h"
#include "glm/fwd.hpp"
#include <memory>
#include <nlohmann/json.hpp>
namespace FooGame
{

#if 1
#define MODEL_PATH    "../../../Assets/Model/viking_room.obj"
#define MODEL_TEXTURE "../../../Assets/Model/viking_room.png"
#else
#define MODEL_PATH    "../../Assets/Model/viking_room.obj"
#define MODEL_TEXTURE "../../Assets/Model/viking_room.png"
#endif

    struct Foo : public ScriptableEntity
    {
            void OnCreate() override { std::cout << "Script Created " << std::endl; }
            void OnDestroy() override { std::cout << "Script Destroy " << std::endl; }
            void Hello()
            {
                if (index <= 10)
                {
                    std::cout << "Hello world " << std::endl;
                }
                index++;
            }
            void OnUpdate(float ts) override { Hello(); }
            u32 index = 0;
    };
    Scene::Scene()
    {
        auto e = CreateEntity();
        glm::mat4{1.0f};
    }

    Scene::~Scene()
    {
    }

    template <typename... Component>
    static void CopyComponent(entt::registry& dst, entt::registry& src,
                              const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        (
            [&]()
            {
                auto view = src.view<Component>();
                for (auto srcEntity : view)
                {
                    entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

                    auto& srcComponent = src.get<Component>(srcEntity);
                    dst.emplace_or_replace<Component>(dstEntity, srcComponent);
                }
            }(),
            ...);
    }

    template <typename... Component>
    static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst,
                              entt::registry& src,
                              const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        CopyComponent<Component...>(dst, src, enttMap);
    }

    template <typename... Component>
    static void CopyComponentIfExists(Entity dst, Entity src)
    {
        (
            [&]()
            {
                if (src.HasComponent<Component>())
                {
                    dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
                }
            }(),
            ...);
    }

    template <typename... Component>
    static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
    {
        CopyComponentIfExists<Component...>(dst, src);
    }

    std::shared_ptr<Scene> Scene::Copy(std::shared_ptr<Scene> other)
    {
        std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

        newScene->m_ViewportWidth  = other->m_ViewportWidth;
        newScene->m_ViewportHeight = other->m_ViewportHeight;

        auto& srcSceneRegistry = other->m_Registry;
        auto& dstSceneRegistry = newScene->m_Registry;
        std::unordered_map<UUID, entt::entity> enttMap;

        // Create entities in new scene
        auto idView = srcSceneRegistry.view<IDComponent>();
        for (auto e : idView)
        {
            UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
            // const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
            Entity newEntity = newScene->CreateEntityWithUUID(uuid, "name");
            enttMap[uuid]    = (entt::entity)newEntity;
        }

        // Copy components (except IDComponent and TagComponent)
        CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

        return newScene;
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
    {
        Entity entity = {m_Registry.create(), this};
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        // auto& tag = entity.AddComponent<TagComponent>();
        // tag.Tag   = name.empty() ? "Entity" : name;

        m_EntityMap[uuid] = entity;

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_EntityMap.erase(entity.GetUUID());
        m_Registry.destroy(entity);
    }

    void Scene::OnUpdate(float ts)
    {
        if (!m_IsPaused || m_StepFrames-- > 0)
        {
            // Update scripts
            {
                m_Registry.view<ScriptComponent>().each(
                    [=](auto entity, auto& nsc)
                    {
                        // TODO: Move to Scene::OnScenePlay
                        if (!nsc.Instance)
                        {
                            nsc.Instance           = nsc.InstantiateScript();
                            nsc.Instance->m_Entity = Entity{entity, this};
                            nsc.Instance->OnCreate();
                        }

                        nsc.Instance->OnUpdate(ts);
                    });
            }
        }
    }

    void Scene::Step(int frames)
    {
        m_StepFrames = frames;
    }

    Entity Scene::DuplicateEntity(Entity entity)
    {
        // std::string name = entity.GetName();
        Entity newEntity = CreateEntity("name");
        CopyComponentIfExists(AllComponents{}, newEntity, entity);
        return newEntity;
    }

    Entity Scene::GetEntityByUUID(UUID uuid)
    {
        if (m_EntityMap.find(uuid) != m_EntityMap.end())
        {
            return {m_EntityMap.at(uuid), this};
        }

        return {};
    }

    void Scene::RenderScene3D(PerspectiveCamera* camera)
    {
        // camera->RecalculateViewMatrix();
        // Renderer3D::BeginScene(*camera);
        // m_Registry.view<TransformComponent, MeshRendererComponent>().each(
        //     [=](auto transform, auto& comp)
        //     {
        //         for (auto& id : comp.PtrModel->GetIds())
        //         {
        //             Renderer3D::DrawModel(id, transform.GetTransform());
        //         }
        //     });
        // Renderer3D::EndScene();
    }
    void Scene::RenderScene2D(OrthographicCamera* camera)
    {
        Renderer2D::BeginScene(*camera);
        m_Registry.view<TransformComponent, MeshRendererComponent>().each(
            [=](auto transform, auto& comp) {});
        Renderer2D::EndScene();
    }
    void Scene::IMGUI()
    {
        m_Registry.view<TransformComponent>().each(
            [=](auto& transform)
            {
                ImGui::Begin("Transform");
                float scale[3] = {
                    transform.Scale.x,
                    transform.Scale.y,
                    transform.Scale.z,
                };
                float pos[3] = {
                    transform.Translation.x,
                    transform.Translation.y,
                    transform.Translation.z,
                };

                ImGui::SliderFloat3("Scale", scale, 0.1f, 20.0f);
                transform.Scale.x = scale[0];
                transform.Scale.y = scale[1];
                transform.Scale.z = scale[2];
                ImGui::SliderFloat3("Translation", pos, 0.1f, 20.0f);
                transform.Translation.x = pos[0];
                transform.Translation.y = pos[1];
                transform.Translation.z = pos[2];
                ImGui::End();
            });
    }

    template <typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        static_assert(sizeof(T) == 0);
    }

    template <>
    void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
    {
    }

    template <>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
    {
    }

    template <>
    void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
    {
    }
    template <>
    void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity,
                                                        MeshRendererComponent& component)
    {
    }
    std::unique_ptr<Scene> LoadSceneFromJson(std::ifstream& stream)
    {
        using json       = nlohmann::json;
        json data        = json::parse(stream);
        int id           = data["id"];
        std::string name = data["name"];
        return std::make_unique<Scene>();
    }

}  // namespace FooGame
