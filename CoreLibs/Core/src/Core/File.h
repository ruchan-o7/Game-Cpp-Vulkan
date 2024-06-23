#pragma once
#include <string>
#include <filesystem>
#include "src/Base.h"
namespace FooGame
{
    class File
    {
        public:
            static std::filesystem::path GetCWD();
            static std::filesystem::path GetAssetDirectory();

        public:
            static void OpenFileDialog(List<std::filesystem::path>& outPaths);
            static void OpenMessageBox(const char* msg);

        public:
            static std::string ExtractFileName(const std::string& pathStr);
            static std::string ExtractFileName(const std::filesystem::path& path);
            static std::string ExtractExtensionName(const std::string& path);
            static std::string ExtractExtensionName(const std::filesystem::path& path);
    };
}  // namespace FooGame