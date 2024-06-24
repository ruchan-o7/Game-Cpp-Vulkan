#pragma once
#include "../Base.h"

enum class AssetStatus
{
    NONE,
    WORKING,
    READY,
    FAILED,
};
template <typename AssetType>
struct AssetContainer
{
        AssetStatus Status;
        AssetType Asset;
        std::string Path;
};
namespace FooGame::Asset
{

    struct TextureInfo
    {
            String Name;
            float factor[4];
    };

    enum class AlphaMode
    {
        Opaque,
        Transparent
    };
    struct FMaterial
    {
            String Name;
            TextureInfo BaseColorTexture;

            String MetallicTextureName;
            float MetallicFactor;

            String RoughnessTextureName;
            float RoughnessFactor;

            TextureInfo EmissiveTexture;

            AlphaMode alphaMode = AlphaMode::Opaque;
            float AlphaCutOff   = 0.5f;
            bool DoubleSided    = false;
    };
    enum class TextureFormat
    {
        RGBA8,
        RGB8,
    };
    struct FImage
    {
            String Name;
            size_t Size;
            u32 Width;
            u32 Height;
            u8 ChannelCount;
            TextureFormat Format;
            //     List<unsigned char> Data;
            std::string BufferPath;
    };
    struct FMesh
    {
            String Name;
            String MaterialName;
            size_t VertexCount;
            size_t IndicesCount;
            List<float> Vertices;
            List<u32> Indices;
            size_t TotalSize;
    };
    struct FModel
    {
            String Name;
            u32 MeshCount = 1;
            List<FMesh> Meshes;
    };

}  // namespace FooGame::Asset