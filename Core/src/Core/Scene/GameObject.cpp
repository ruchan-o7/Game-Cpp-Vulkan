#include "GameObject.h"
namespace FooGame
{

    int GameObject::NextId = 0;
    GameObject::GameObject(const String& name) : m_Name(name), m_Id(NextId++)
    {
    }

}  // namespace FooGame
