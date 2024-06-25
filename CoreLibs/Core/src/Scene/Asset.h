#pragma once
#include "../Base.h"
#include "../Core/UUID.h"
namespace FooGame::Asset
{
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
            Shared<AssetType> Asset;
            String Name;
            UUID Id;
    };

    struct TextureInfo
    {
            UUID id = 0;
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
            // String NormalTextureName;
            UUID NormalTextureId;

            // String MetallicTextureName;
            UUID MetallicTextureId = 0;
            float MetallicFactor;

            UUID RoughnessTextureId = 0;
            // String RoughnessTextureName;
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
            std::string BufferPath;
    };
    struct FMesh
    {
            String Name;
            String MaterialName;
            UUID MaterialId;
            size_t VertexCount;
            size_t IndicesCount;
            List<float> Vertices;
            List<u32> Indices;
            size_t TotalSize;
            // UUID Id;
    };
    struct FModel
    {
            String Name;
            u32 MeshCount = 1;
            List<FMesh> Meshes;
            // UUID Id;
    };

}  // namespace FooGame::Asset
