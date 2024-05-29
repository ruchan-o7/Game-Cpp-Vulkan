#pragma once

#include <memory>
#include "EditorScene.h"
namespace FooGame
{
    class EditorSceneDeserializer
    {
        public:
            EditorSceneDeserializer()  = default;
            ~EditorSceneDeserializer() = default;
            std::unique_ptr<EditorScene> DeSerialize(
                const std::string& scenePath);
    };

}  // namespace FooGame
