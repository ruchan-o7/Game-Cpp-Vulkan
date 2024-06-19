#include "File.h"
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

}  // namespace FooGame