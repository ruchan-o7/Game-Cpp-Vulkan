#pragma once
#include "../Base.h"
#include "Asset.h"
#include <json.hpp>
namespace FooGame
{
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
            nlohmann::json Serialize(const Asset::FModel& model);
            Asset::FModel DeSerialize(const nlohmann::json& json);
    };
}  // namespace FooGame