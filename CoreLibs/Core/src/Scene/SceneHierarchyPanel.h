#pragma once
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
            void DrawMaterial();
            void DrawImage();
            void DrawModel();
            void DrawAssets();
            void RefreshAssetFiles();
            void DrawAssetProperty();

        private:
            Scene* m_pScene;
            Entity m_SelectionContext;
            u64 m_SelectedMaterial = 0;
    };
}  // namespace FooGame
