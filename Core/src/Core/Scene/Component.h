#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Core/Graphics/Model.h"
namespace FooGame
{
    class GameObject;
    struct Component
    {
        public:
            Component()          = default;
            virtual ~Component() = default;
            void SetOwner(GameObject* owner) { this->m_Owner = owner; }
            GameObject* GetOwner() const { return m_Owner; }

        private:
            GameObject* m_Owner = nullptr;
    };
    struct MeshRendererComponent : public Component
    {
            MeshRendererComponent(Shared<Model> model) : m_Model(model) {}
            Shared<Model> GetModel() const { return m_Model; }

        private:
            Shared<Model> m_Model;
    };
}  // namespace FooGame
