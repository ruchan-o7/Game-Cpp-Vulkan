#pragma once
#include <pch.h>
#include "../../Base.h"
#include "../../Core/UUID.h"
namespace FooGame
{
    struct DrawPrimitive
    {
            u32 FirstIndex  = 0;
            u32 IndexCount  = 0;
            UUID MaterialId = 0;
    };

    struct Mesh
    {
        public:
            Mesh()  = default;
            ~Mesh() = default;

            u64 MaterialId;
            std::string Name;
            uint32_t RenderId;
            List<DrawPrimitive> DrawSpecs;
    };

}  // namespace FooGame
