#pragma once
#include "../../Base.h"
#include "../../Core/UUID.h"

namespace ENGINE_NAMESPACE
{
    struct Mesh
    {
        struct Primitive
        {
                u32 FirstIndex  = 0;
                u32 IndexCount  = 0;
                UUID MaterialId = 0;
        };
        String Name = String("Unnamed Mesh");
        List<Primitive> Primitives;
        glm::mat4 Transform;
    };
}