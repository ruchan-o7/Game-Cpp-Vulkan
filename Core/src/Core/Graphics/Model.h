#pragma once
#include "Core/Backend/Vertex.h"
#include "Core/Core/Base.h"
namespace FooGame
{
    class Mesh
    {
        public:
            Mesh(List<Vertex>&& vertices, List<u32>&& indices);
            List<Vertex> m_Vertices;
            List<u32> m_Indices;
    };
    class Model
    {
        public:
            static Shared<Model> LoadModel(const String& path);
            List<Mesh> GetMeshes() const { return m_Meshes; }
            Model(List<Mesh>&& meshes);
            glm::vec3 Position = {0.0f, 0.0f, 0.0f};

        private:
            List<Mesh> m_Meshes;
    };

}  // namespace FooGame
