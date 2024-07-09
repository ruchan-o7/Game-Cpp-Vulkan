#include "File.h"
#include <pch.h>
#include "../Base.h"
#include "../Core/Application.h"
#include "../Core/Window.h"
#include <commdlg.h>
#include <combaseapi.h>
#include <objbase.h>
#include <winerror.h>
#include <shobjidl.h>
#include <winscard.h>
#include <winuser.h>
namespace FooGame
{
    static std::filesystem::path m_SceneBasePath, m_AssetPath, m_ModelsPath, m_ImagesPath,
        m_MaterialsPath;
    void File::SetSceneBasePath(const std::filesystem::path& p)
    {
        m_SceneBasePath = p;
        m_AssetPath     = GetCWD() / m_SceneBasePath / "Assets";
        m_ModelsPath    = m_AssetPath / "Models";
        m_ImagesPath    = m_AssetPath / "Images";
        m_MaterialsPath = m_AssetPath / "Materials";
#define CREATE_IF_NOT_EXISTS(x)                        \
    if (!std::filesystem::exists(x.string()))          \
    {                                                  \
        std::filesystem::create_directory(x.string()); \
    }
        CREATE_IF_NOT_EXISTS(m_SceneBasePath);
        CREATE_IF_NOT_EXISTS(m_AssetPath);
        CREATE_IF_NOT_EXISTS(m_ModelsPath);
        CREATE_IF_NOT_EXISTS(m_ImagesPath);
        CREATE_IF_NOT_EXISTS(m_MaterialsPath);
#undef CREATE_IF_NOT_EXISTS
    }
    std::filesystem::path File::GetAssetPath()
    {
        return m_AssetPath;
    }
    std::filesystem::path File::GetModelsPath()
    {
        return m_ModelsPath;
    }
    std::filesystem::path File::GetImagesPath()
    {
        return m_ImagesPath;
    }
    std::filesystem::path File::GetMaterialsPath()
    {
        return m_MaterialsPath;
    }
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
    void File::OpenMessageBox(const char* msg)
    {
        MessageBoxW(NULL, (const wchar_t*)msg, L"File Path", MB_OK);
    }

    String File::OpenFileDialog(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260]     = {0};
        CHAR currentDir[256] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner   = Application::Get().GetWindow().GetWin32NativeHandle();
        ofn.lpstrFile   = szFile;
        ofn.nMaxFile    = sizeof(szFile);
        if (GetCurrentDirectoryA(256, currentDir))
        {
            ofn.lpstrInitialDir = currentDir;
        }
        ofn.lpstrFilter  = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }

        return std::string();
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
    void File::WriteJsonData(std::filesystem::path& path, const String& extension,
                             const nlohmann::json& data)
    {
        path.replace_extension(extension);
        std::ofstream o{path};
        DEFER(o.close(););
        o << std::setw(4) << data << std::endl;
    }
}  // namespace FooGame
