#pragma once
#include <string>
namespace FooGame
{

    struct NormalTextureInfo
    {
            std::string Name;
    };

    struct PbrMetallicRoughness
    {
            double BaseColorFactor[4] = {1.0, 1.0, 1.0, 1.0};
            std::string BaseColorTextureName;
            std::string BaseColorTexturePath;

            double MetallicFactor{1.0};
            double RoughnessFactor{1.0};
            std::string MetallicRoughnessTextureName;
            std::string MetallicRoughnessTexturePath;
    };
    struct Material
    {
            std::string Name;
            NormalTextureInfo NormalTexture;
            PbrMetallicRoughness PbrMat;
            bool fromGlb = false;
    };

}  // namespace FooGame
