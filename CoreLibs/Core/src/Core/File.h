#pragma once
#include <string>
#include <filesystem>
namespace FooGame
{
    class File
    {
        public:
            static std::string ExtractFileName(const std::string& pathStr);
            static std::string ExtractFileName(const std::filesystem::path& path);
            static std::string ExtractExtensionName(const std::string& path);
            static std::string ExtractExtensionName(const std::filesystem::path& path);
    };
}  // namespace FooGame