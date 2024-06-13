#pragma once
#include <cstdint>
#include <string>
namespace FooGame
{
    struct Material
    {
            int32_t BaseColorTextureIndex  = 0;
            int32_t MetallicRoughnessIndex = 0;
            int32_t NormalTextureIndex     = 0;
            std::string Name;
    };
    struct Material2
    {
            std::string Name;
            std::string AlbedoMap;
    };
}  // namespace FooGame
