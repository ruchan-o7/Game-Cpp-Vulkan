#include "Model.h"
namespace FooGame
{
    Model::Model(std::vector<Mesh>&& meshes) : m_Meshes(std::move(meshes))
    {
    }

}  // namespace FooGame
