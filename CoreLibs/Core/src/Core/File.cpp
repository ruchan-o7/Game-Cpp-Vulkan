#include "File.h"
#include <filesystem>
namespace FooGame
{

    std::string File::ExtractFileName(const std::string& pathStr)
    {
        return std::filesystem::path(pathStr).filename().string();
    }
    std::string File::ExtractFileName(const std::filesystem::path& path)
    {
        return path.filename().string();
    }

    std::string File::ExtractExtensionName(const std::string& path)
    {
        return std::filesystem::path(path).extension().string();
    }
    std::string File::ExtractExtensionName(const std::filesystem::path& path)
    {
        return path.extension().string();
    }
    std::filesystem::path File::GetCWD()
    {
        static std::filesystem::path Cwd;
        if (Cwd.empty())
        {
            Cwd = std::filesystem::current_path();
        }
        return Cwd;
    }
    std::filesystem::path File::GetAssetDirectory()
    {
        static std::filesystem::path AssetDir;
        if (AssetDir.empty())
        {
            AssetDir = GetCWD() / "Assets";
        }
        return AssetDir;
    }

}  // namespace FooGame