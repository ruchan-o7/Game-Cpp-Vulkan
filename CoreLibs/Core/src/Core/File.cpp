#include "File.h"
#include <cstddef>
#include <filesystem>
#include "../Core/Window.h"
#include "src/Base.h"
#include <GLFW/glfw3.h>
#include <combaseapi.h>
#include <objbase.h>
#include <winerror.h>
#include <shobjidl.h>
#include <winscard.h>
#include <winuser.h>
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
    void File::OpenMessageBox(const char* msg)
    {
        MessageBoxW(NULL, (const wchar_t*)msg, L"File Path", MB_OK);
    }

    void File::OpenFileDialog(List<std::filesystem::path>& outPaths)
    {
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr))
        {
            IFileOpenDialog* pFileOpen;

            // Create the FileOpenDialog object.
            hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog,
                                  reinterpret_cast<void**>(&pFileOpen));

            if (SUCCEEDED(hr))
            {
                // Show the Open dialog box.
                hr = pFileOpen->Show(NULL);

                // Get the file name from the dialog box.
                if (SUCCEEDED(hr))
                {
                    IShellItem* pItem;
                    hr = pFileOpen->GetResult(&pItem);
                    if (SUCCEEDED(hr))
                    {
                        PWSTR pszFilePath;
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                        // Display the file name to the user.
                        if (SUCCEEDED(hr))
                        {
                            outPaths.push_back({pszFilePath});
                            CoTaskMemFree(pszFilePath);
                        }
                        pItem->Release();
                    }
                }
                pFileOpen->Release();
            }
            CoUninitialize();
        }
    }
}  // namespace FooGame