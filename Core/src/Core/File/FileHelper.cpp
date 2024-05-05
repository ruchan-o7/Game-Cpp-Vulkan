#include "FileHelper.h"
#include "pch.h"
#include <fstream>
namespace FooGame
{
    List<char> ReadFile(const std::string& file_path)
    {
        std::ifstream file(file_path, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            // TODO: Handle without exception
            throw std::runtime_error("failed to open file!");
        }

        size_t file_size = (size_t)file.tellg();
        List<char> buffer(file_size);

        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(file_size));

        file.close();

        return buffer;
    }
}  // namespace FooGame
