#pragma once
#include <Engine.h>
#include <memory>
namespace FooGame
{
    std::unique_ptr<Model> LoadModel(const std::string& path);
}  // namespace FooGame
