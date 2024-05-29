#include "Entity.h"
namespace FooGame
{
    Entity::Entity(entt::entity handle, Scene* scene)
        : m_EntityHandle(handle), m_Scene(scene)
    {
    }

}  // namespace FooGame
