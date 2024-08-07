#pragma once
#include <entt/entt.hpp>
#include "../Core/UUID.h"
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

            [[nodiscard]] Entity CreateEntity(const std::string& name = std::string());
            Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
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
            void RenderScene();
            void IMGUI();

            const UUID GetId() const { return m_Id; }

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
            std::string m_Name;
            UUID m_Id;

            friend class Entity;
            friend class SceneSerializer;
            friend class EditorLayer;
            friend class SceneHierarchyPanel;
    };
}  // namespace FooGame
