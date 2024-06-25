#include "Model.h"
namespace FooGame
{
    Model::Model(std::vector<Mesh>&& meshes) : Meshes(std::move(meshes))
    {
    }

}  // namespace FooGame
