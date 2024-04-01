#include "Include/Core/Utils/PlatfromUtils_Windows.h"

#include "Include/MnemosyConfig.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Window.h"
#include "Include/Core/Log.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <windows.h>
#include <commdlg.h>
#include <shobjidl_core.h>
#include <shlobj_core.h>
#include <Shellapi.h>
#include <objidl.h>
#include <TCHAR.H>
#include <oleidl.h>


#include <string>
#include <filesystem>


#ifdef MNEMOSY_PLATFORM_WINDOWS




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

	void FileDialogs::StartDrag()
	{
		// NOTHING IS WORKING 
		// but check out this maybe https://stackoverflow.com/questions/449872/how-drag-and-drop-of-files-is-done



		// unregister too
		HRESULT r = CoInitialize(nullptr);

		if (FAILED(r)) {
			MNEMOSY_TRACE("CoInitialize faild");
		}


		std::vector<std::string> filesToDrag;

		std::filesystem::path p1 = std::filesystem::path(R"(D:/ExportTests/DropTestMat_albedo_sRGB.png)");
		std::filesystem::path p2 = std::filesystem::path(R"(D:/ExportTests/DropTestMat_normal_gl_raw.png)");

		filesToDrag.push_back(p1.generic_string());
		filesToDrag.push_back(p2.generic_string());


		// calculate the total size of memeory we need to allocate
		UINT uBuffSize = 0;

		for (int i = 0; i < filesToDrag.size(); i++) {
			uBuffSize += lstrlen(filesToDrag[i].c_str()) + 1;

		}
		uBuffSize = sizeof(DROPFILES) + sizeof(TCHAR) * (uBuffSize + 1);


		// Allocate memory from the heap for the DROPFILES struct.
		HGLOBAL hGlobal = GlobalAlloc(GHND, uBuffSize);;
		if (NULL == hGlobal)
			return;
				

		DROPFILES* pDrop = (DROPFILES*)GlobalLock(hGlobal);

		if (NULL == pDrop) {
			GlobalFree(hGlobal);
			return;
		}


		// Fill in the DROPFILES struct.
		pDrop->pFiles = sizeof(DROPFILES);

//#ifdef _UNICODE
//		// If we're compiling for Unicode, set the Unicode flag in the struct to
//		// indicate it contains Unicode strings.
//		pDrop->fWide = TRUE;
//#endif


		//Now we can copy all of the filenames into memory, and then unlock the buffer.


		// Copy all the filenames into memory after
		// the end of the DROPFILES struct.
		TCHAR* pszBuff;
		pszBuff = (TCHAR*)(LPBYTE(pDrop) + sizeof(DROPFILES));

		for (int i = 0; i < filesToDrag.size(); i++) {
			//uBuffSize += lstrlen(filesToDrag[i].c_str()) + 1;
			lstrcpy(pszBuff, (LPCTSTR)filesToDrag[i].c_str());
			pszBuff = 1 + _tcschr(pszBuff, '\0');

		}
		GlobalUnlock(hGlobal);

		// The next step is to construct a COleDataSource object and put our data into it.We also need a FORMATETC
		//  struct that describes the clipboard format(CF_HDROP) and how the data is stored(an HGLOBAL).

		FORMATETC etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

		// Put the data in the data source.
		//COleDataSource datasrc;
		//datasrc.CacheGlobalData(CF_HDROP, hGlobal, &etc);


		//DROPEFFECT dwEffect;

		//dwEffect = datasrc.DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE);



		//STGMEDIUM stg;
		//stg.hGlobal = hGlobal;
		//stg.tymed = TYMED_HGLOBAL;
		//stg.pUnkForRelease = nullptr;



		 // Create data object
		//IDataObject* pDataObject = new FileDataObject(filesToDrag);
		//
		////pDataObject->GetData(&etc, &stg);
		//
		//
		////pDataObject->SetData(&etc,&stg,FALSE);
		// // Start drag operation
		//DWORD dwEffect = 0;
		//HRESULT hr =  DoDragDrop(pDataObject, nullptr, DROPEFFECT_COPY, &dwEffect); 
		//
		//
		//if (FAILED(hr)) {
		//
		//	DWORD dwErrorCode = HRESULT_CODE(hr);
		//	  HRESULT_FROM_NT(dwErrorCode);
		//	MNEMOSY_TRACE("Error code: {}",dwErrorCode);
		//}
		//
		//pDataObject->Release();
		
	}

} // mnemosy::core

#endif // !MNEMOSY_PLATFORM_WINDOW