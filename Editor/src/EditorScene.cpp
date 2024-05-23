#include <Engine.h>
#include "EditorScene.h"
#include <Log.h>
namespace FooGame
{
    std::unique_ptr<EditorScene> EditorScene::LoadScene(std::ifstream& stream)
    {
        return std::unique_ptr<EditorScene>();
    }
    EditorScene::~EditorScene()
    {
        FOO_EDITOR_TRACE("Editor Scene deleting...");
        for (auto& t : Textures)
        {
            DestroyImage(t.get());
            t.reset();
        }
        for (auto& m : Meshes)
        {
            // m.MeshPtr.reset();
        }
    }

}  // namespace FooGame
