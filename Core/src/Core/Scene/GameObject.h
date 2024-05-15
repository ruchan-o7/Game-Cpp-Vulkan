#pragma once
#include <typeindex>
#include <unordered_map>
#include "Core/Core/Base.h"
#include "glm/gtx/quaternion.hpp"
namespace FooGame
{
    class Component;
    struct Transform
    {
            Transform()                 = default;
            Transform(const Transform&) = default;
            Transform(const glm::vec3& pos) : Position(pos) {}

            glm::vec3 Position{0.0f, 0.0f, 0.0f};
            glm::vec3 Rotation{0.0f, 0.0f, 0.0f};
            glm::vec3 Scale{1.0f, 1.0f, 1.0f};

            glm::mat4 GetTransform() const
            {
                glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
                return glm::translate(glm::mat4(1.0f), Position) * rotation *
                       glm::scale(glm::mat4(1.0f), Scale);
            }
    };
    class GameObject
    {
        public:
            GameObject(const String& name);
            GameObject(const GameObject&) = default;

        public:
            Transform Transform;

        public:
            template <typename T>
            void AddComponent(Shared<T> comp)
            {
                m_Components[typeid(T)] = comp;
                comp->SetOwner(this);
            }
            template <typename T>
            void RemoveComponent()
            {
                m_Components.erase(typeid(T));
            }
            template <typename T>
            Shared<T> GetComponent()
            {
                for (auto& comps : m_Components)
                {
                    if (comps.first == typeid(T))
                    {
                        return std::reinterpret_pointer_cast<T>(comps.second);
                    }
                }
                return nullptr;
            }

            int GetId() const { return m_Id; }
            String GetName() const;
            void SetEnabled(bool value) { m_Enabled = value; }
            bool IsEnabled() const { return m_Enabled; }

        private:
            String m_Name  = "GameObject";
            bool m_Enabled = true;
            int m_Id;
            static int NextId;
            using ComponentMap =
                std::unordered_map<std::type_index, Shared<Component>>;
            ComponentMap m_Components;
    };
}  // namespace FooGame
