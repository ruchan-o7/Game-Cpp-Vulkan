#include "Model.h"
#include "../Core/VulkanTexture.h"
namespace FooGame
{
    Model::Model(std::vector<Mesh>&& meshes) : m_Meshes(std::move(meshes))
    {
    }

}  // namespace FooGame
