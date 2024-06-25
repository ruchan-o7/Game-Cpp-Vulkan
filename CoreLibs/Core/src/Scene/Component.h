#pragma once
#include <Log.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../Core/UUID.h"
#include "../Base.h"
#include "Asset.h"
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
            UUID ModelId;
            MeshRendererComponent()                             = default;
            MeshRendererComponent(const MeshRendererComponent&) = default;
    };
    class ScriptableEntity;
    struct ScriptComponent;

    struct ScriptFactory
    {
            ScriptableEntity* Instance = nullptr;
            ScriptableEntity* (*InstantiateScript)();
            void (*DestroyScript)(ScriptComponent*, String name);
    };

    namespace Script
    {
        using ScriptRegistery = Hashmap<String, ScriptFactory>;
        inline ScriptRegistery& GetRegistry()
        {
            static ScriptRegistery registery;
            return registery;
        }

        inline bool HasScriptExists(const String& tag)
        {
            auto& r = GetRegistry();
            if (r.find(tag) != r.end())
            {
                return true;
            }
            return false;
        }

        inline bool RegisterScriptFactory(const String& factoryTag, ScriptFactory factory)
        {
            if (HasScriptExists(factoryTag))
            {
                // FOO_CORE_WARN("Script already registered: {0}", factoryTag);
                return false;
            }
            auto res =
                GetRegistry().insert(ScriptRegistery::value_type{factoryTag, factory}).second;
            if (!res)
            {
                // FOO_CORE_WARN("Script could not inserted: {0}", factoryTag);
                return false;
            }
            return true;
        }

        inline ScriptFactory* GetScript(const String& tag)
        {
            if (HasScriptExists(tag))
            {
                return &GetRegistry()[tag];
            }
            FOO_CORE_WARN("Script could not found bruh: {0}", tag);
            return nullptr;
        }

    }  // namespace Script

    struct ScriptComponent
    {
            Hashmap<String, ScriptFactory*> Scripts;

            void Bind(const String& name) { Scripts[name] = Script::GetScript(name); }
            void RemoveScript(const String& name)
            {
                Scripts[name]->DestroyScript(this, name);
                Scripts.erase(name);
            }
    };

#define REGISTER_SCRIPT(S)                                                              \
    const inline bool ScriptRegistery__##S{Script::RegisterScriptFactory(               \
        #S, {nullptr, []() { return static_cast<ScriptableEntity*>(new Script::S()); }, \
             [](ScriptComponent* sc, String name)                                       \
             {                                                                          \
                 auto& s = sc->Scripts[name];                                           \
                 delete s->Instance;                                                    \
                 s->Instance = nullptr;                                                 \
             }})};

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
