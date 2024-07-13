#pragma once
#include <pch.h>
#include "../Base.h"
#include "../Core/UUID.h"
#include "../Config.h"
#include "src/Engine/Core/VulkanTexture.h"
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
            AssetType Asset;
    };
    template <>
    struct AssetContainer<VulkanTexture>
    {
            AssetStatus Status;
            Shared<VulkanTexture> Asset;
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
            UUID Id;
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
    static FMaterial* CreateNewMaterial()
    {
        auto* m                  = new FMaterial;
        m->Name                  = "New Material";
        m->BaseColorTexture.Name = DEFAULT_TEXTURE_NAME;
        m->BaseColorTexture.id   = DEFAULT_TEXTURE_ID;

        return m;
    }
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
            UUID Id;
    };

}  // namespace FooGame::Asset
