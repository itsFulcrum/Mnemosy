#include "Include/Core/Utils/PlatfromUtils_Windows.h"

#include "Include/MnemosyConfig.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Window.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <string>
#include <filesystem>


#ifdef MNEMOSY_PLATFORM_WINDOWS

#include <windows.h>
#include <commdlg.h>
#include <shobjidl_core.h>



namespace mnemosy::core
{
	// functions return empty string if it didn't work so check for empty string pls

	// filter can be like this ->     "png (*.png)\0*.png"
	
	std::string FileDialogs::OpenFile(const char* filter)
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
			return ofn.lpstrFile;
		}

		return std::string();
	}

	std::string FileDialogs::SaveFile(const char* filter)
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
			return ofn.lpstrFile;
		}
		
		return std::string();
	}

	std::string FileDialogs::SelectFolder(const char* filter) {

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

					//hacky string convertion because windows is stupid
					std::filesystem::path convert = g_path;
					return convert.generic_string();

					psi->Release();
				}
			}
			pfd->Release();
		}

		return std::string();
	}

} // mnemosy::core

#endif // !MNEMOSY_PLATFORM_WINDOWS