#include <Engine.h>
#include "EditorScene.h"
#include <Log.h>
#include "src/Geometry/AssetLoader.h"
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
            AssetLoader::DestroyTexture(*t.get());
            t.reset();
        }
        for (auto& m : MeshDatas)
        {
            // m.MeshPtr.reset();
        }
    }

}  // namespace FooGame
