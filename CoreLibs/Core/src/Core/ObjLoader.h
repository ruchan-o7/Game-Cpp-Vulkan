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
            ObjLoader(const std::filesystem::path& path, const std::string& modelName,
                      const std::string& materialName);
            std::unique_ptr<ObjModel> LoadModel();

        private:
            std::filesystem::path m_Path;
            std::string m_ModelName, m_MaterialName;
    };
}  // namespace FooGame