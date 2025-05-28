#include "AssetSerializer.h"
#include "Asset.h"
#include "json.hpp"
namespace FooGame
{
    nlohmann::json MaterialSerializer::Serialize(const Asset::FMaterial& mat)
    {
        using json = nlohmann::json;
        json j;
        j["name"]                  = mat.Name;
        j["id"]                    = (u64)mat.Id;
        j["baseColorTexture"]      = json::object({
            {    "id", (u64)mat.BaseColorTexture.id},
            {"factor",  mat.BaseColorTexture.factor}
        });
        j["metallicColorTexture"]  = json::object({
            {    "id", (u64)mat.MetallicTextureId},
            {"factor",         mat.MetallicFactor}
        });
        j["roughnessColorTexture"] = json::object({
            {    "id", (u64)mat.RoughnessTextureId},
            {"factor",         mat.RoughnessFactor}
        });
        j["emissiveColorTexture"]  = json::object({
            {    "id", (u64)mat.EmissiveTexture.id},
            {"factor",  mat.EmissiveTexture.factor}
        });
        switch (mat.alphaMode)
        {
            case Asset::AlphaMode::Opaque:
            {
                j["alphaMode"] = "OPAQUE";
                break;
            }
            case Asset::AlphaMode::Transparent:
                j["alphaMode"] = "TRANSPARENT";
                break;
        }
        j["alphaCutOff"] = mat.AlphaCutOff;
        j["doubleSided"] = mat.DoubleSided;
        return j;
    }
    Asset::FMaterial MaterialSerializer::DeSerialize(const nlohmann::json& json)
    {
        Asset::FMaterial mat;
        mat.Name                       = json["name"];
        mat.Id                         = json["id"].get<u64>();
        mat.BaseColorTexture.id        = json["baseColorTexture"]["id"].get<u64>();
        mat.BaseColorTexture.factor[0] = json["baseColorTexture"]["factor"][0];
        mat.BaseColorTexture.factor[1] = json["baseColorTexture"]["factor"][1];
        mat.BaseColorTexture.factor[2] = json["baseColorTexture"]["factor"][2];
        mat.BaseColorTexture.factor[3] = json["baseColorTexture"]["factor"][3];

        mat.MetallicTextureId = json["metallicColorTexture"]["id"].get<u64>();
        mat.MetallicFactor    = 1.0;  // TODO: Change it after deserialize properly
                                      // json["metallicColorTexture"]["factor"];

        mat.RoughnessTextureId = json["roughnessColorTexture"]["id"].get<u64>();
        mat.RoughnessFactor    = 1.0;  // TODO: json["roughnessColorTexture"]["factor"];

        mat.EmissiveTexture.id        = json["emissiveColorTexture"]["id"].get<u64>();
        mat.EmissiveTexture.factor[0] = json["emissiveColorTexture"]["factor"][0];
        mat.EmissiveTexture.factor[1] = json["emissiveColorTexture"]["factor"][1];
        mat.EmissiveTexture.factor[2] = json["emissiveColorTexture"]["factor"][2];
        mat.EmissiveTexture.factor[3] = json["emissiveColorTexture"]["factor"][3];

        std::string alphaMode = json["alphaMode"];
        if (alphaMode == "OPAQUE")
        {
            mat.alphaMode = Asset::AlphaMode::Opaque;
        }
        else if (alphaMode == "TRANSPARENT")
        {
            mat.alphaMode = Asset::AlphaMode::Transparent;
        }

        mat.AlphaCutOff = json["alphaCutOff"];
        mat.DoubleSided = json["doubleSided"];
        return mat;
    }
    nlohmann::json ImageSerializer::Serialize(const Asset::FImage& image)
    {
        using json = nlohmann::json;
        json j;
        j["id"]           = (u64)image.Id;
        j["name"]         = image.Name;
        j["size"]         = image.Size;
        j["width"]        = image.Width;
        j["height"]       = image.Height;
        j["channelCount"] = image.ChannelCount;
        switch (image.Format)
        {
            case Asset::TextureFormat::RGBA8:
                j["format"] = "RGBA8";
                break;
            case Asset::TextureFormat::RGB8:
                j["format"] = "RGB8";
                break;
        }

        return j;
    }
    Asset::FImage ImageSerializer::DeSerialize(const nlohmann::json& json)
    {
        Asset::FImage i;
        i.Name         = json["name"];
        i.Id           = json["id"].get<u64>();
        i.Size         = json["size"];
        i.Width        = json["width"];
        i.Height       = json["height"];
        i.ChannelCount = json["channelCount"];
        i.Format       = Asset::TextureFormat::RGBA8;
        return i;
    }
    nlohmann::json ModelSerializer::Serialize(const Asset::FModel& model)
    {
        using json = nlohmann::json;
        json j;
        j["name"]         = model.Name;
        j["meshCount"]    = model.MeshCount;
        j["vertexCount"]  = model.VertexCount;
        j["indicesCount"] = model.IndicesCount;
        j["totalSize"]    = model.TotalSize;
        j["vertices"]     = model.Vertices;
        j["indices"]      = model.Indices;
        j["id"]           = (u64)model.Id;
        for (auto& mesh : model.Meshes)
        {
            json m;
            m["name"] = mesh.Name;
            for (const auto& pr : mesh.Primitives)
            {
                json pJ;
                pJ["materialId"] = (u64)pr.MaterialId;
                pJ["firstIndex"] = pr.FirstIndex;
                pJ["indexCount"] = pr.IndexCount;
                m["primitives"].push_back(pJ);
            }
            float matrices[4][4];
            for (i32 i = 0; i < 4; i++)
            {
                for (i32 j = 0; j < 4; j++)
                {
                    matrices[i][j] = mesh.Transform[i][j];
                }
            }
            m["transform"] = matrices;
            j["meshes"].push_back(m);
        }
        return j;
    }
    Asset::FModel ModelSerializer::DeSerialize(const nlohmann::json& json)
    {
        Asset::FModel m;
        m.Name         = json["name"];
        m.MeshCount    = json["meshCount"];
        m.TotalSize    = json["totalSize"];
        m.VertexCount  = json["vertexCount"];
        m.IndicesCount = json["indicesCount"];
        m.Id           = json["id"].get<u64>();
        List<float> ve = json["vertices"];
        List<u32> in   = json["indices"];
        m.Vertices     = std::move(ve);
        m.Indices      = std::move(in);
        m.Meshes.reserve(json["meshes"].size());

        for (auto& mj : json["meshes"])
        {
            Asset::FMesh mesh;
            String name = mj["name"].get<String>();
            if (!name.empty())
            {
                mesh.Name = name;
            }
            List<List<float>> transformVec = mj["transform"];
            glm::mat4 transform;
            for (i32 i = 0; i < 4; i++)
            {
                for (i32 j = 0; j < 4; j++)
                {
                    transform[i][j] = transformVec[i][j];
                }
            }
            mesh.Transform = transform;
            mesh.Primitives.reserve(mj["primitives"].size());
            for (const auto& p : mj["primitives"])
            {
                Asset::DrawPrimitive pr;
                pr.IndexCount = p["indexCount"];
                pr.FirstIndex = p["firstIndex"];
                pr.MaterialId = p["materialId"].get<u64>();
                mesh.Primitives.push_back(pr);
            }
            m.Meshes.emplace_back(std::move(mesh));
        }

        return m;
    }
}  // namespace FooGame
