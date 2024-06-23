#include "ObjLoader.h"
#include <tiny_obj_loader.h>
#include "../Engine/Geometry/Mesh.h"
#include "../Engine/Geometry/Material.h"
#include <Log.h>
#include <memory>

namespace FooGame
{
    static std::vector<Material> ProcessMaterial(
        const std::vector<tinyobj::material_t>& objMaterials);
    ObjLoader::ObjLoader(const std::filesystem::path& path) : m_Path(path)
    {
    }
    std::unique_ptr<ObjModel> ObjLoader::LoadModel() const
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> objMaterials;
        std::string warn, err;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Mesh> meshes;

        auto objPath        = m_Path.string();
        auto objBasePath    = m_Path.parent_path();
        auto objBasePathStr = objBasePath.string();

        if (!tinyobj::LoadObj(&attrib, &shapes, &objMaterials, &warn, &err, objPath.c_str(),
                              objBasePathStr.c_str()))
        {
            FOO_ENGINE_ERROR("Model could not loaded {0}", m_Path.string());
            return nullptr;
        }
        auto materials = ProcessMaterial(objMaterials);

        for (const auto& shape : shapes)
        {
            Mesh mesh;
            mesh.Name = shape.name;

            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};
                vertex.Position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };
                if (index.texcoord_index >= 0)
                {
                    vertex.TexCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }
                else
                {
                    vertex.TexCoord = {1.0f, 1.0f};
                }
                vertex.Color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };  // im not sure
                if (index.normal_index >= 0)
                {
                    vertex.Normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };  // im not sure
                }
                else
                {
                    vertex.Normal = {1.0f, 1.0f, 1.0f};
                }
                mesh.m_Vertices.emplace_back(std::move(vertex));
                mesh.m_Indices.push_back(mesh.m_Indices.size());
            }
            meshes.emplace_back(std::move(mesh));
        }
        ObjModel* model  = new ObjModel;
        model->Materials = std::move(materials);
        model->Meshes    = std::move(meshes);
        model->Name      = m_Path.filename().string();
        enum TEXTURE_INDICES
        {
            ALBEDO    = 0,
            METALIC   = 1,
            ROUGHNESS = 2,
            NORMAL    = 3,
        };
        // model->textureIndices = {ALBEDO, METALIC, ROUGHNESS, NORMAL};
        return std::unique_ptr<ObjModel>(model);
    }
    std::vector<Material> ProcessMaterial(const std::vector<tinyobj::material_t>& objMaterials)
    {
        std::vector<Material> materials;
        for (const auto& mat : objMaterials)
        {
            Material material;
            material.Name               = mat.name;
            material.fromGlb            = false;
            material.NormalTexture.Name = mat.bump_texname;

            auto& pbr = material.PbrMat;

            pbr.BaseColorFactor[0]   = mat.diffuse[0];
            pbr.BaseColorFactor[1]   = mat.diffuse[1];
            pbr.BaseColorFactor[2]   = mat.diffuse[2];
            pbr.BaseColorFactor[3]   = 1.0;
            pbr.BaseColorTextureName = mat.diffuse_texname;

            pbr.MetallicRoughnessTextureName = mat.roughness_texname;
            pbr.MetallicFactor               = mat.metallic;
            pbr.RoughnessFactor              = mat.roughness;
            materials.emplace_back(std::move(material));
        }
        return materials;
    }

}  // namespace FooGame