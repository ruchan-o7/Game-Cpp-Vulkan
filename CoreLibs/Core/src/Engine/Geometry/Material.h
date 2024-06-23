#pragma once
#include "../../Base.h"
namespace FooGame
{

    struct NormalTextureInfo
    {
            String Name;
    };

    struct PbrMetallicRoughness
    {
            double BaseColorFactor[4] = {1.0, 1.0, 1.0, 1.0};
            String BaseColorTextureName;
            String BaseColorTexturePath;

            double MetallicFactor{1.0};
            double RoughnessFactor{1.0};
            String MetallicRoughnessTextureName;
            String MetallicRoughnessTexturePath;
    };
    struct Material
    {
            String Name;
            NormalTextureInfo NormalTexture;
            PbrMetallicRoughness PbrMat;
            bool fromGlb = false;
    };

}  // namespace FooGame
