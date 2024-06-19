#include "Mesh.h"
#include "../Defines.h"
#include "../Engine/VulkanCheckResult.h"
#include "../Engine/Backend.h"
#include "../Core/RenderDevice.h"
namespace FooGame
{

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
        : m_Vertices(vertices), m_Indices(indices)
    {
    }
    Mesh::Mesh()
    {
    }
    Mesh::~Mesh()
    {
        m_Vertices.clear();
        m_Indices.clear();
    }
    Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
        : m_Vertices(std::move(vertices)), m_Indices(std::move(indices))
    {
    }
    Mesh::Mesh(Mesh&& other)
    {
        m_Vertices       = std::move(other.m_Vertices);
        m_Indices        = std::move(other.m_Indices);
        m_DescriptorSets = std::move(other.m_DescriptorSets);
        MeshPrimitives   = std::move(other.MeshPrimitives);
        materialData     = std::move(other.materialData);
        RenderId         = other.RenderId;
        other.m_Vertices.clear();
        other.m_Indices.clear();
        other.m_DescriptorSets.clear();
        other.MeshPrimitives = {};
        other.materialData   = {};
        other.RenderId       = -1;
    }

}  // namespace FooGame
