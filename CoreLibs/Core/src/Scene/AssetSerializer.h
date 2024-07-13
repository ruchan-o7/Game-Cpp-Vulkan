#pragma once
#include "Asset.h"
#include <json.hpp>
namespace FooGame
{
    struct Model;
    class MaterialSerializer
    {
        public:
            nlohmann::json Serialize(const Asset::FMaterial& mat);
            Asset::FMaterial DeSerialize(const nlohmann::json& json);
    };
    class ImageSerializer
    {
        public:
            nlohmann::json Serialize(const Asset::FImage& image);
            Asset::FImage DeSerialize(const nlohmann::json& json);
    };
    class ModelSerializer
    {
        public:
            nlohmann::json Serialize(const Model& model);
            Model DeSerialize(const nlohmann::json& json);
    };
}  // namespace FooGame