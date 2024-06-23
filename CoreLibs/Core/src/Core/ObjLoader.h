#pragma once
#include <memory>
#include <string>
#include "File.h"

namespace FooGame
{
    struct Model;
    struct Mesh;
    struct Material;
    struct ObjModel
    {
            std::vector<Mesh> Meshes;
            std::vector<Material> Materials;
            std::string Name;
    };
    class ObjLoader
    {
        public:
            ObjLoader(const std::filesystem::path& path);
            std::unique_ptr<ObjModel> LoadModel() const;
            const std::filesystem::path& GetPath() const { return m_Path; }

        private:
            std::filesystem::path m_Path;
    };
}  // namespace FooGame