#pragma once
#include <unordered_map>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../Core/UUID.h"
#include "../Base.h"
namespace FooGame
{
    class Model;
    struct IDComponent
    {
            UUID ID;

            IDComponent()                   = default;
            IDComponent(const IDComponent&) = default;
    };
    struct TagComponent
    {
            String Tag;

            TagComponent()                    = default;
            TagComponent(const TagComponent&) = default;
            TagComponent(const String& tag) : Tag(tag) {}
    };

    struct TransformComponent
    {
            glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
            glm::vec3 Rotation    = {0.0f, 0.0f, 0.0f};
            glm::vec3 Scale       = {1.0f, 1.0f, 1.0f};

            TransformComponent()                          = default;
            TransformComponent(const TransformComponent&) = default;
            TransformComponent(const glm::vec3& translation) : Translation(translation) {}
            glm::mat4 operator()() { return GetTransform(); }
            glm::mat4 GetTransform() const
            {
                glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

                return glm::translate(glm::mat4(1.0f), Translation) * rotation *
                       glm::scale(glm::mat4(1.0f), Scale);
            }
    };
    struct MeshRendererComponent
    {
            List<String> MeshNames;
            String ModelName = "Empty Component";
            String ModelPath;
            String MaterialName;
            MeshRendererComponent()                             = default;
            MeshRendererComponent(const MeshRendererComponent&) = default;
            MeshRendererComponent(String name, String modelPath, String materialName)
                : ModelName(name), ModelPath(modelPath), MaterialName(materialName)
            {
            }
    };
    class ScriptableEntity;

    struct ScriptComponent
    {
            struct ScriptFactory
            {
                    ScriptableEntity* Instance = nullptr;
                    ScriptableEntity* (*InstantiateScript)();
                    void (*DestroyScript)(ScriptComponent*, String name);
            };
            Hashmap<String, ScriptFactory> Scripts;

            template <typename T>
            void Bind(const String& name)
            {
                ScriptFactory factory{};
                factory.InstantiateScript = []()
                { return static_cast<ScriptableEntity*>(new T()); };
                factory.DestroyScript = [](ScriptComponent* nsc, String name)
                {
                    auto& script = nsc->Scripts[name];

                    delete script.Instance;
                    script.Instance = nullptr;
                };
                Scripts[name] = factory;
            }
            void RemoveScript(const String& name)
            {
                Scripts[name].DestroyScript(this, name);
                Scripts.erase(name);
            }
    };
    class Camera;
    struct CameraComponent
    {
            Camera* pCamera;
            bool Primary          = true;
            bool FixedAspectRatio = false;

            CameraComponent()                       = default;
            CameraComponent(const CameraComponent&) = default;
    };
    template <typename... Component>
    struct ComponentGroup
    {
    };

    using AllComponents = ComponentGroup<TransformComponent, TagComponent, MeshRendererComponent,
                                         ScriptComponent, CameraComponent>;
}  // namespace FooGame
