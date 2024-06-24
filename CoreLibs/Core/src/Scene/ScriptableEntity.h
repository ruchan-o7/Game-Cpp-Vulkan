#pragma once
#include "Entity.h"
namespace FooGame
{

    class ScriptableEntity
    {
        public:
            virtual ~ScriptableEntity() {}

            template <typename T>
            T& GetComponent()
            {
                return m_Entity.GetComponent<T>();
            }

        protected:
            virtual void OnCreate() {}
            virtual void OnDestroy() {}
            virtual void OnUpdate(float ts) {}

        private:
            Entity m_Entity;
            friend class Scene;
    };
}  // namespace FooGame
