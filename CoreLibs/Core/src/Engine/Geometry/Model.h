#pragma once
#include "Mesh.h"
#include "../../Base.h"
namespace ENGINE_NAMESPACE
{
    struct Model
    {
            Model() = default;

            UUID Id;
            String Name = "Unnamed model";
            String LastChangeTime;
            String CreateTime;
            List<Mesh> Meshes;
            String ModelPath;

            struct Statistics
            {
                    size_t ByteSize       = 0;
                    size_t VertexCount    = 0;
                    size_t IndexCount     = 0;
                    size_t MeshCount      = 0;
                    size_t PrimitiveCount = 0;
            } Stats;
    };

}  // namespace ENGINE_NAMESPACE
