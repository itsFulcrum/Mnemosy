
#include "Include/Core/Utils/PlatfromUtils_Windows.h"

#include "Include/MnemosyConfig.h"

#ifdef MNEMOSY_PLATFORM_WINDOWS


#include "Include/MnemosyEngine.h"
#include "Include/Core/Window.h"
#include "Include/Core/Log.h"



#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <windows.h>
#include <commdlg.h>
#include <shobjidl_core.h>
//#include <shlobj_core.h>
#include <Shellapi.h>





namespace mnemosy::core
{	
	std::filesystem::path FileDialogs::OpenFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = {0};
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(&MnemosyEngine::GetInstance().GetWindow().GetWindow()); // file explorer window need to know parent window
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{

			//std::string str = std::string(ofn.lpstrFile);
			//return std::filesystem::u8path(str);

			return std::filesystem::u8path(ofn.lpstrFile);
		}

		return std::filesystem::path();
	}

	std::filesystem::path FileDialogs::SaveFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(&MnemosyEngine::GetInstance().GetWindow().GetWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{

			//std::string str = std::string(ofn.lpstrFile);

			//return std::filesystem::u8path(str);

			return std::filesystem::u8path(ofn.lpstrFile);
		}
		
		return std::filesystem::path();
	}

	std::filesystem::path FileDialogs::SelectFolder(const char* filter) {

		IFileDialog* pfd;
		LPWSTR g_path;

		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)))) {
			DWORD dwOptions;
			if (SUCCEEDED(pfd->GetOptions(&dwOptions))) {
				pfd->SetOptions(dwOptions | FOS_PICKFOLDERS); 
			}
			if (SUCCEEDED(pfd->Show(NULL))) {

				IShellItem* psi;
				if (SUCCEEDED(pfd->GetResult(&psi))) {

					if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &g_path))) {

						MessageBox(NULL, "GetIDListName() failed", NULL, NULL);
						return std::string();
					}
					psi->Release();

					//hacky string convertion because windows is stupid

					return std::filesystem::path(g_path);
				}
			}
			pfd->Release();
		}

		return std::filesystem::path();
	}

	bool FileDialogs::OpenFolderAt(const std::filesystem::path& folderPath) {

		if (std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath)) {

			ShellExecute(NULL, "open", folderPath.generic_string().c_str(), NULL, NULL, SW_SHOWNORMAL);
			return true;

		}
		else {
			return false;
		}
	}


} // mnemosy::core

#endif // !MNEMOSY_PLATFORM_WINDOW