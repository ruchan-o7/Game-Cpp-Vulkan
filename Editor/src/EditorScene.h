#pragma once
#include <Core.h>
namespace FooGame
{
    class Mesh;
    struct Texture2D;
    struct MeshData
    {
            TransformComponent Transform;
            // Shared<Mesh> MeshPtr;
            Shared<Model> ModelPtr;
            // uint32_t Id;
            uint32_t TextureIndex;
            ~MeshData() = default;
    };
    struct ModelData
    {
    };
    class EditorScene
    {
        public:
            static std::unique_ptr<EditorScene> LoadScene(
                std::ifstream& stream);
            EditorScene() = default;
            ~EditorScene();

            List<MeshData> MeshDatas;
            List<std::shared_ptr<Texture2D>> Textures;
            String Name;
            int Id;
    };
}  // namespace FooGame
