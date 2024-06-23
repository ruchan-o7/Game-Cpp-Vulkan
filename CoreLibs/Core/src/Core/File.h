#pragma once
#include <string>
#include <filesystem>
#include "src/Base.h"
namespace FooGame
{
    class File
    {
        public:
            static void SetSceneBasePath(const std::filesystem::path& p);

        public:
            static std::filesystem::path GetCWD();
            static std::filesystem::path GetAssetPath();
            static std::filesystem::path GetModelsPath();
            static std::filesystem::path GetImagesPath();
            static std::filesystem::path GetMaterialsPath();
            static std::filesystem::path GetSceneBasePath();

        public:
            static void OpenFileDialog(List<std::filesystem::path>& outPaths);
            static void OpenMessageBox(const char* msg);

        public:
            static std::string ExtractFileName(const std::string& pathStr);
            static std::string ExtractFileName(const std::filesystem::path& path);
            static std::string ExtractExtensionName(const std::string& path);
            static std::string ExtractExtensionName(const std::filesystem::path& path);

        private:
    };
}  // namespace FooGame