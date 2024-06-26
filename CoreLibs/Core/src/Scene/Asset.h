#pragma once
#include "../Base.h"
#include "../Core/UUID.h"
#include "../Config.h"
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
            UUID id = DEFAULT_TEXTURE_ID;
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
            UUID NormalTextureId;

            UUID MetallicTextureId = DEFAULT_TEXTURE_ID;
            float MetallicFactor;

            UUID RoughnessTextureId = DEFAULT_TEXTURE_ID;
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
    /// Texture basically
    struct FImage
    {
            String Name;
            size_t Size;
            u32 Width;
            u32 Height;
            u8 ChannelCount;
            TextureFormat Format;
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
    };
    struct FModel
    {
            String Name;
            u32 MeshCount = 1;
            List<FMesh> Meshes;
    };

}  // namespace FooGame::Asset
