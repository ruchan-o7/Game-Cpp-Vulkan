#pragma once
#include <cstdint>
#include <string>
namespace FooGame
{
    struct Material
    {
            uint32_t BaseColorTextureIndex  = 0;
            uint32_t MetallicRoughnessIndex = 0;
            uint32_t NormalTextureIndex     = 0;
            std::string Name;
    };
}  // namespace FooGame