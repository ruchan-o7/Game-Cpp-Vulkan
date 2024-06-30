#include "ObjLoader.h"
#include <tiny_obj_loader.h>
#include "../Engine/Geometry/Mesh.h"
#include "../Config.h"
#include "../Scene/Asset.h"
#include "../Engine/Geometry/Vertex.h"
#include <Log.h>

namespace FooGame
{
    static List<Asset::FMaterial> ProcessMaterial(const List<tinyobj::material_t>& objMaterials);
    ObjLoader::ObjLoader(const std::filesystem::path& path) : m_Path(path)
    {
    }
    std::unique_ptr<ObjModel> ObjLoader::LoadModel() const
    {
        tinyobj::attrib_t attrib;
        List<tinyobj::shape_t> shapes;
        List<tinyobj::material_t> objMaterials;
        String warn, err;
        List<Vertex> vertices;
        List<u32> indices;
        List<Mesh> meshes;

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
            mesh.Name       = shape.name;
            u32 firstIndex  = static_cast<u32>(vertices.size());
            u32 vertexStart = static_cast<u32>(indices.size());
            u32 indexCount  = 0;

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
                vertices.emplace_back(std::move(vertex));
                indices.push_back(indices.size());
            }
            indexCount += static_cast<u32>(indices.size());
            DrawPrimitive p{};
            p.FirstIndex = firstIndex;
            p.IndexCount = indexCount;
            p.MaterialId = DEFAULT_MATERIAL_ID;
            mesh.DrawSpecs.emplace_back(std::move(p));
            meshes.emplace_back(std::move(mesh));
        }
        ObjModel* model  = new ObjModel;
        model->Materials = std::move(materials);
        model->Meshes    = std::move(meshes);
        model->Name      = m_Path.filename().string();
        model->Vertices  = std::move(vertices);
        model->Indices   = std::move(indices);

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
    List<Asset::FMaterial> ProcessMaterial(const List<tinyobj::material_t>& objMaterials)
    {
        List<Asset::FMaterial> materials;
        for (const auto& mat : objMaterials)
        {
            Asset::FMaterial material;
            material.Name                       = mat.name;
            material.BaseColorTexture.factor[0] = mat.diffuse[0];
            material.BaseColorTexture.factor[1] = mat.diffuse[1];
            material.BaseColorTexture.factor[2] = mat.diffuse[2];
            material.BaseColorTexture.factor[3] = 1.0;
            material.BaseColorTexture.Name      = mat.diffuse_texname;

            // material.NormalTextureName = mat.bump_texname;

            // material.MetallicTextureName = mat.roughness_texname;
            material.MetallicFactor = mat.metallic;

            material.RoughnessFactor = mat.roughness;
            // material.RoughnessTextureName = mat.roughness_texname;

            materials.emplace_back(std::move(material));
        }
        return materials;
    }

}  // namespace FooGame
