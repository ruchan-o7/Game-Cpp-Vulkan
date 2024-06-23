#pragma once
#include <string_view>
#include "Entity.h"
namespace FooGame
{
    class Scene;
    class SceneHierarchyPanel
    {
        public:
            SceneHierarchyPanel(Scene* context);
            void SetContext(Scene* context);
            void OnImgui();
            void SetSelectedEntity(Entity e);
            Entity GetSelectedEntity();

        private:
            template <typename T>
            void DisplayAddComponentEntry(const std::string& entryName);
            void DrawEntityNode(Entity entity);
            void DrawComponents(Entity entity);
            void DrawMaterial(std::string_view material);

        private:
            Scene* m_pScene;
            Entity m_SelectionContext;
            const char* m_SelectedMaterial = nullptr;
    };
}  // namespace FooGame