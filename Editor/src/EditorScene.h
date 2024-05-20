#pragma once
#include <memory>
#include <vector>
#include <Scene/Component.h>
namespace FooGame
{
    struct Texture2D;
    struct MeshData
    {
            TransformComponent Transform;
            std::shared_ptr<Mesh> MeshPtr;
            uint32_t Id;
            uint32_t TextureIndex;
    };
    class EditorScene
    {
        public:
            static std::unique_ptr<EditorScene> LoadScene(
                std::ifstream& stream);
            EditorScene() = default;
            ~EditorScene();

            std::vector<MeshData> Meshes;
            std::vector<std::shared_ptr<Texture2D>> Textures;
            std::string Name;
            int Id;
    };
}  // namespace FooGame
