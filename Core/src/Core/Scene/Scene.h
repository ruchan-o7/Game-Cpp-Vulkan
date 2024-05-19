#pragma once
#include "../Core/UUID.h"
#include <entt/entt.hpp>
#include <entt/entity/registry.hpp>
// #include <Engine.h>
namespace FooGame
{
    class Camera;
    class Entity;
    class PerspectiveCamera;
    class OrthographicCamera;
    class Scene
    {
        public:
            Scene();
            ~Scene();

            static std::shared_ptr<Scene> Copy(std::shared_ptr<Scene> other);

            [[nodiscard]] Entity CreateEntity(
                const std::string& name = std::string());
            Entity CreateEntityWithUUID(
                UUID uuid, const std::string& name = std::string());
            void DestroyEntity(Entity entity);

            void OnUpdate(float ts);
            Entity DuplicateEntity(Entity entity);

            Entity FindEntityByName(std::string_view name);
            Entity GetEntityByUUID(UUID uuid);

            Entity GetPrimaryCameraEntity();

            bool IsRunning() const { return m_IsRunning; }
            bool IsPaused() const { return m_IsPaused; }

            void SetPaused(bool paused) { m_IsPaused = paused; }

            void Step(int frames = 1);

            template <typename... Components>
            auto GetAllEntitiesWith()
            {
                return m_Registry.view<Components...>();
            }
            void RenderScene3D(PerspectiveCamera* camera);
            void RenderScene2D(OrthographicCamera* camera);
            void IMGUI();

        private:
            template <typename T>
            void OnComponentAdded(Entity entity, T& component);

        private:
            entt::registry m_Registry;
            uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
            bool m_IsRunning = false;
            bool m_IsPaused  = false;
            int m_StepFrames = 0;

            std::unordered_map<UUID, entt::entity> m_EntityMap;

            friend class Entity;
            // friend class SceneSerializer;
            // friend class SceneHierarchyPanel;
    };
    // class Scene
    // {
    //         DELETE_COPY(Scene);
    //
    //     public:
    //         Scene() = default;
    //         virtual void OnCreate(){};
    //         virtual void OnUpdate(float deltaTime){};
    //         virtual void OnLateUpdate(float deltaTime){};
    //         virtual void OnFixedUpdate(){};
    //         virtual void OnRender(){};
    //         virtual void OnUI(){};
    //         virtual void OnEvent(Event& event){};
    //
    //     public:
    // };
    // class SampleScene final : public Scene
    // {
    //         DELETE_COPY(SampleScene);
    //
    //     public:
    //         SampleScene();
    //         void OnCreate() override;
    //         void OnUpdate(float deltaTime) override;
    //         void OnUI() override;
    //         void OnRender() override;
    //         Shared<Model> m_V1;
    //         Shared<Model> m_V2;
    //         std::unordered_map<String, std::shared_ptr<GameObject>> Objects;
    //
    //     private:
    //         PerspectiveCamera m_Camera;
    //         OrthographicCamera m_Ortho{0, 1, 0, -1};
    // };
}  // namespace FooGame
