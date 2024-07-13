#pragma once
#include "../Scene/Asset.h"
#include "../Base.h"
#include "../Engine/Geometry/Mesh.h"

namespace FooGame
{
    struct Model;
    struct Mesh;
    struct Material;
    struct Vertex;
    struct ObjModel
    {
            List<Mesh> Meshes;
            List<Asset::FMaterial> Materials;
            String Name;
            List<Vertex> Vertices;
            List<u32> Indices;
    };
    class ObjLoader
    {
        public:
            ObjLoader(const std::filesystem::path& path);
            Unique<ObjModel> LoadModel() const;
            const std::filesystem::path& GetPath() const { return m_Path; }

        private:
            std::filesystem::path m_Path;
    };
}  // namespace FooGame
