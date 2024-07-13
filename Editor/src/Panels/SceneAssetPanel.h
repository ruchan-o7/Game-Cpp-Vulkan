#pragma once
#include <Core.h>
#include <filesystem>

namespace FooGame
{
    class AssetPanel
    {
        public:
            explicit AssetPanel(const std::filesystem::path& workingDirectory);
            void OnImGui();
            static void Refresh();
    };
}  // namespace FooGame