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
        j["baseColorTexture"]      = json::object({
            {  "name",   mat.BaseColorTexture.Name},
            {"factor", mat.BaseColorTexture.factor}
        });
        j["metallicColorTexture"]  = json::object({
            {  "name",   mat.BaseColorTexture.Name},
            {"factor", mat.BaseColorTexture.factor}
        });
        j["roughnessColorTexture"] = json::object({
            {  "name",   mat.BaseColorTexture.Name},
            {"factor", mat.BaseColorTexture.factor}
        });
        j["emissiveColorTexture"]  = json::object({
            {  "name",   mat.BaseColorTexture.Name},
            {"factor", mat.BaseColorTexture.factor}
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
        mat.Name = json["name"];

        mat.BaseColorTexture.Name      = json["baseColorTexture"]["name"];
        mat.BaseColorTexture.factor[0] = json["baseColorTexture"]["factor"][0];
        mat.BaseColorTexture.factor[1] = json["baseColorTexture"]["factor"][1];
        mat.BaseColorTexture.factor[2] = json["baseColorTexture"]["factor"][2];
        mat.BaseColorTexture.factor[3] = json["baseColorTexture"]["factor"][3];

        mat.MetallicTextureName = json["metallicColorTexture"]["name"];
        mat.MetallicFactor      = 1.0;  // TODO Change it after deserialize properly
                                        // json["metallicColorTexture"]["factor"];

        mat.RoughnessTextureName = json["roughnessColorTexture"]["name"];
        mat.RoughnessFactor      = 1.0;  // TODO json["roughnessColorTexture"]["factor"];

        mat.EmissiveTexture.Name      = json["emissiveColorTexture"]["name"];
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
        j["data"] = image.Data;

        return j;
    }
    Asset::FImage ImageSerializer::DeSerialize(const nlohmann::json& json)
    {
        Asset::FImage i;
        i.Name         = json["name"];
        i.Size         = json["size"];
        i.Width        = json["width"];
        i.Height       = json["height"];
        i.ChannelCount = json["channelCount"];
        std::string l  = json["layout"];
        if (l == "RGBA8")
        {
            i.Format = Asset::TextureFormat::RGBA8;
        }
        else if (l == "RGB8")
        {
            i.Format = Asset::TextureFormat::RGB8;
        }
        List<unsigned char> data = json["data"];
        i.Data                   = std::move(data);
        return i;
    }
    nlohmann::json ModelSerializer::Serialize(const Asset::FModel& model)
    {
        using json = nlohmann::json;
        json j;
        j["name"]      = model.Name;
        j["meshCount"] = model.MeshCount;
        for (auto& mesh : model.Meshes)
        {
            json m;
            m["name"]         = mesh.Name;
            m["materialName"] = mesh.MaterialName;
            m["vertexCount"]  = mesh.VertexCount;
            m["indicesCount"] = mesh.IndicesCount;
            m["totalSize"]    = mesh.TotalSize;
            m["vertices"]     = mesh.Vertices;
            m["indices"]      = mesh.Indices;
            j["meshes"].push_back(m);
        }
        return j;
    }
    Asset::FModel ModelSerializer::DeSerialize(const nlohmann::json& json)
    {
        Asset::FModel m;
        m.Name      = json["name"];
        m.MeshCount = json["meshCount"];

        for (auto& mj : json["meshes"])
        {
            Asset::FMesh mesh;
            mesh.Name         = mj["name"];
            mesh.MaterialName = mj["materialName"];
            mesh.VertexCount  = mj["vertexCount"];
            mesh.IndicesCount = mj["indicesCount"];
            mesh.TotalSize    = mj["totalSize"];
            List<float> ve    = mj["vertices"];
            List<u32> in      = mj["indices"];
            mesh.Vertices     = std::move(ve);
            mesh.Indices      = std::move(in);
            m.Meshes.emplace_back(std::move(mesh));
        }

        return m;
    }
}  // namespace FooGame