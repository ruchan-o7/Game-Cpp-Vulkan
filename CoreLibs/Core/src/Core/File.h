#pragma once
#include <string>
#include <filesystem>
#include "../Base.h"
#include <json.hpp>
namespace FooGame
{
    class File
    {
            using Path = std::filesystem::path;

        public:
            static void SetSceneBasePath(const Path& p);

        public:
            static Path GetCWD();
            static Path GetAssetPath();
            static Path GetModelsPath();
            static Path GetImagesPath();
            static Path GetMaterialsPath();
            static Path GetSceneBasePath();

            static void WriteJsonData(Path& path, const String& extension,
                                      const nlohmann::json& data);
            static void ReadJsonData(const Path& path, nlohmann::json& out);

        public:
            static void OpenFileDialog(List<Path>& outPaths);
            static String OpenFileDialog(const char* filter = nullptr);
            static void OpenMessageBox(const char* msg);

        public:
            static std::string ExtractFileName(const std::string& pathStr);
            static std::string ExtractFileName(const std::filesystem::path& path);
            static std::string ExtractExtensionName(const std::string& path);
            static std::string ExtractExtensionName(const std::filesystem::path& path);

        private:
    };
}  // namespace FooGame
