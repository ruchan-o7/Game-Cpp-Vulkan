#include "Scene.h"
#include "../Scene/Entity.h"
#include "../Scene/ScriptableEntity.h"
#include <imgui.h>
#include "../Scene/Component.h"
#include <cstddef>
#include <memory>
#include "src/Engine/Camera/Camera.h"
#include "src/Engine/Engine/Renderer2D.h"
#include "src/Engine/Engine/Renderer3D.h"
#include "src/Scene/Scene.h"
#include <nlohmann/json.hpp>
namespace FooGame
{

    Scene::Scene()
    {
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
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag   = name.empty() ? "Entity" : name;

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
                        for (auto& [name, sc] : nsc.Scripts)
                        {
                            if (!sc.Instance)
                            {
                                sc.Instance           = sc.InstantiateScript();
                                sc.Instance->m_Entity = Entity{entity, this};
                                sc.Instance->OnCreate();
                            }
                            sc.Instance->OnUpdate(ts);
                        }
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
        std::string name = entity.GetName();
        Entity newEntity = CreateEntity(name);
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
    Entity Scene::GetPrimaryCameraEntity()
    {
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            const auto& camera = view.get<CameraComponent>(entity);
            if (camera.Primary)
            {
                return Entity{entity, this};
            }
        }
        return {};
    }
    void Scene::RenderScene()
    {
        Camera* mainCamera = nullptr;
        glm::mat4 cameraTransform;
        {
            auto view = m_Registry.view<TransformComponent, CameraComponent>();
            for (auto entity : view)
            {
                auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
                if (camera.Primary)
                {
                    mainCamera      = camera.pCamera;
                    cameraTransform = transform.GetTransform();
                }
            }
        }
        if (mainCamera)
        {
            Renderer3D::BeginScene(*mainCamera);
        }
        m_Registry.view<TransformComponent, MeshRendererComponent>().each(
            [=](auto& transform, auto& comp) {
                Renderer3D::DrawModel(comp.ModelName, comp.MaterialName, transform.GetTransform());
            });
    }
    void Scene::IMGUI()
    {
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
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
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
    template <>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
    }

}  // namespace FooGame
