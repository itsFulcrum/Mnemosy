#include "Include/Core/Utils/PlatfromUtils_Windows.h"

#include "Include/MnemosyConfig.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Window.h"
#include "Include/Core/Log.h"

#include "Include/Core/Utils/DropManager_Windows.h"

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

#include <comdef.h>

#include <string>
#include <filesystem>


#ifdef MNEMOSY_PLATFORM_WINDOWS


// microstoft com introduction
// https://learn.microsoft.com/en-us/windows/win32/learnwin32/memory-allocation-in-com


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

	void FileDialogs::RegisterDropManager(DropManager* dropManager) {
		
		HRESULT hr;
		// unregister too
		{
			hr = CoInitialize(NULL);
			CheckComError("RegisterDropManager::CoInitialize()",hr);

			hr = OleInitialize(NULL);
			CheckComError("RegisterDropManager::OleInitialize()",hr);
		}
		

		
		hr = RegisterDragDrop(glfwGetWin32Window(&MnemosyEngine::GetInstance().GetWindow().GetWindow()), static_cast<IDropTarget*>(dropManager));
		CheckComError("RegisterDropManager:: RegisterDragDrop()", hr);

		// TODO Deallocate!!
		DropSourceClassFactory* DropSourceFactory = new DropSourceClassFactory();
		DataObjectClassFactory* DataObjectFactory = new DataObjectClassFactory();

		DWORD class_reg;
		hr = CoRegisterClassObject(CLSID_FileDataObject, (IUnknown*)DataObjectFactory,CLSCTX_INPROC_SERVER,REGCLS_MULTIPLEUSE, &class_reg);
		CheckComError("CoRegisterClassObject FileDataObject", hr);

		DWORD class_reg2;
		hr = CoRegisterClassObject(CLSID_DropSource, (IUnknown*)DropSourceFactory,CLSCTX_INPROC_SERVER,REGCLS_MULTIPLEUSE, &class_reg2);
		CheckComError("CoRegisterClassObject DropSource", hr);


		RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
		RegisterClipboardFormat(CFSTR_FILECONTENTS);
		RegisterClipboardFormat(CFSTR_FILENAMEMAP);
	}

	void FileDialogs::StartDrag()
	{
		// NOTHING IS WORKING 
		// but check out this maybe https://stackoverflow.com/questions/449872/how-drag-and-drop-of-files-is-done

		MNEMOSY_DEBUG("PlatformUtils::StartDrag:");

		// TODO Clear this
		std::vector<std::string> filesToDrag;
	
		std::filesystem::path p1 = std::filesystem::path(R"(D:/Drop1.png)");
		std::filesystem::path p2 = std::filesystem::path(R"(D:/Drop2.png)");

		filesToDrag.push_back(p1.generic_string());
		filesToDrag.push_back(p2.generic_string());
		{

		//std::filesystem::directory_entry d1 = std::filesystem::directory_entry(filesToDrag[0]);
		//if (d1.exists()) {
		//	if (d1.is_regular_file()) {
		//		MNEMOSY_TRACE("Dragfiles are valid");
		//	}
		//}


		// calculate the total size of memeory we need to allocate
		//UINT uBuffSize = 0;
		//
		//for (int i = 0; i < filesToDrag.size(); i++) {
		//	uBuffSize += lstrlen(filesToDrag[i].c_str()) + 1;
		//
		//}
		//uBuffSize = sizeof(DROPFILES) + sizeof(TCHAR) * (uBuffSize + 1);
		//
		//
		//// Allocate memory from the heap for the DROPFILES struct.
		//HGLOBAL hGlobal = GlobalAlloc(GHND, uBuffSize);;
		//if (NULL == hGlobal)
		//	return;
		//		
		//
		//DROPFILES* pDrop = (DROPFILES*)GlobalLock(hGlobal);
		//
		//if (NULL == pDrop) {
		//	GlobalFree(hGlobal);
		//	return;
		//}
		//
		//
		//// Fill in the DROPFILES struct.
		//pDrop->pFiles = sizeof(DROPFILES);

//#ifdef _UNICODE
//		// If we're compiling for Unicode, set the Unicode flag in the struct to
//		// indicate it contains Unicode strings.
//		pDrop->fWide = TRUE;
//#endif


		//Now we can copy all of the filenames into memory, and then unlock the buffer.


		// Copy all the filenames into memory after
		// the end of the DROPFILES struct.
		//TCHAR* pszBuff;
		//pszBuff = (TCHAR*)(LPBYTE(pDrop) + sizeof(DROPFILES));
		//
		//for (int i = 0; i < filesToDrag.size(); i++) {
		//	//uBuffSize += lstrlen(filesToDrag[i].c_str()) + 1;
		//	lstrcpy(pszBuff, (LPCTSTR)filesToDrag[i].c_str());
		//	pszBuff = 1 + _tcschr(pszBuff, '\0');
		//
		//}
		//GlobalUnlock(hGlobal);

		// The next step is to construct a COleDataSource object and put our data into it.We also need a FORMATETC
		//  struct that describes the clipboard format(CF_HDROP) and how the data is stored(an HGLOBAL).


		// Put the data in the data source.
		//COleDataSource datasrc;
		//datasrc.CacheGlobalData(CF_HDROP, hGlobal, &etc);


		//DROPEFFECT dwEffect;

		//dwEffect = datasrc.DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE);

		}


		FORMATETC* etc = new FORMATETC();// = { CF_HDROP, NULL, DVASPECT_DOCPRINT, -1, TYMED_HGLOBAL };
		etc->cfFormat = CF_HDROP;
		etc->dwAspect = DVASPECT_DOCPRINT;
		etc->tymed = TYMED_HGLOBAL;

		STGMEDIUM* stg = new STGMEDIUM();// nullptr;
		//stg.hGlobal = hGlobal;
		//stg.tymed = TYMED_HGLOBAL;
		//stg.pUnkForRelease = nullptr;

		HRESULT hr;
		

		IDropSource* pDropSource;
		hr = CoCreateInstance(CLSID_DropSource,NULL, CLSCTX_INPROC_SERVER, IID_IDropSource,reinterpret_cast<void**>(&pDropSource));
		CheckComError("CoCreateInstance for DropSource",hr);

		 // Create data object
		FileDataObject* pDataObject;
		hr = CoCreateInstance(CLSID_FileDataObject, NULL, CLSCTX_INPROC_SERVER, IID_IDataObject, reinterpret_cast<void**>(&pDataObject));
		CheckComError("CoCreateInstance for FileDataObject",hr);


		pDataObject->Init(filesToDrag);

		hr = pDataObject->GetData(etc, stg);
		CheckComError("FileDataObject::GetData:",hr);



		// // Start drag operation
		DWORD dwEffect = 0;
		
		hr =  DoDragDrop(pDataObject,pDropSource, DROPEFFECT_COPY, &dwEffect );
		if (SUCCEEDED(hr)) {
			MNEMOSY_INFO("DoDragDrop SUCCESS!!");
		}
		CheckComError("DoDragDrop():",hr);

		
		MNEMOSY_INFO("End of StartDrag");
		
		pDataObject->Release();
		pDropSource->Release();
		
	}



	void FileDataObject::Init(const std::vector<std::string>& filePaths)
	{
		m_filePaths = filePaths;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::QueryInterface(REFIID riid, void** ppvObj) {

		if (!ppvObj) {
			MNEMOSY_TRACE("IDataObject::QuearyInterface: E_INVALIDARG");
			return E_INVALIDARG;
		}

		*ppvObj = NULL;
		if (riid == IID_IUnknown || riid == CLSID_FileDataObject || riid == IID_IDataObject)
		{
			// Increment the reference count and return the pointer.
			*ppvObj = (LPVOID)this;
			AddRef();
			MNEMOSY_TRACE("IDataObject::QuearyInterface: NOERROR");
			return NOERROR;

		}
		
		MNEMOSY_TRACE("IDataObject::QuearyInterface:E_NOINTERFACE");
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG __stdcall) FileDataObject::AddRef()
	{

		InterlockedIncrement(&m_cRef);
		return m_cRef;

		MNEMOSY_TRACE("IDataObject::AddRef: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(ULONG __stdcall) FileDataObject::Release()
	{
		// Decrement the object's internal counter.
		MNEMOSY_TRACE("IDataObject::Release: Called");
		ULONG ulRefCount = InterlockedDecrement(&m_cRef);

		if (0 == m_cRef)
		{
			delete this;
		}
		return ulRefCount;



		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::GetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium)
	{


		if (pFormatetc->cfFormat == CF_HDROP && pFormatetc->tymed == TYMED_HGLOBAL) {
			
			// Prepare file drop data
			//
			//std::vector<wchar_t> buffer;
			//for (const auto& filePath : m_filePaths) {
			//
			//	buffer.insert(buffer.end(), filePath.begin(), filePath.end());
			//	buffer.push_back(L'\0');
			//    
			//}
			//buffer.push_back(L'\0');// Double null-terminated
			//
			////
			////// Allocate memory for file drop structure
			//HGLOBAL hGlobal = GlobalAlloc(GHND, sizeof(DROPFILES) + (buffer.size() * sizeof(wchar_t)));

			//
			////
			//DROPFILES* pDropFiles = reinterpret_cast<DROPFILES*>(GlobalLock(hGlobal));
			//pDropFiles->pFiles = sizeof(DROPFILES);
			//pDropFiles->fWide = TRUE; // Indicate Unicode file paths
			//memcpy((char*)pDropFiles + sizeof(DROPFILES), buffer.data(), buffer.size() * sizeof(wchar_t));

			//GlobalUnlock(hGlobal);


			////// Set data medium
			//pmedium->hGlobal = hGlobal;
			//pmedium->tymed = TYMED_HGLOBAL;
			//pmedium->pUnkForRelease = nullptr;

			//return S_OK;

			

			
			UINT uBuffSize = 0;

			for (int i = 0; i < m_filePaths.size(); i++) {
				uBuffSize += lstrlen(m_filePaths[i].c_str()) + 1;

			}
			uBuffSize = sizeof(DROPFILES) + sizeof(TCHAR) * (uBuffSize + 1);
			// Allocate memory from the heap for the DROPFILES struct.
			HGLOBAL hGlobal = GlobalAlloc(GHND, uBuffSize);

			DROPFILES* pDrop;// = reinterpret_cast<DROPFILES*>(GlobalLock(hGlobal));
			pDrop = (DROPFILES*)GlobalLock(hGlobal);


			// Fill in the DROPFILES struct.
			pDrop->pFiles = sizeof(DROPFILES);
			pDrop->fWide = FALSE;





			//#ifdef _UNICODE
			//		// If we're compiling for Unicode, set the Unicode flag in the struct to
			//		// indicate it contains Unicode strings.
			//#endif

					//Now we can copy all of the filenames into memory, and then unlock the buffer.


			// Copy all the filenames into memory after
			// the end of the DROPFILES struct.


			TCHAR* pszBuff;
			pszBuff = (TCHAR*)(LPBYTE(pDrop) + sizeof(DROPFILES));

			for (int i = 0; i < m_filePaths.size(); i++) {
				lstrcpy(pszBuff, (LPCTSTR)m_filePaths[i].c_str());
				pszBuff = 1 + _tcschr(pszBuff, '\0');

			}

			GlobalUnlock(hGlobal);

			//// Set data medium
			pmedium->hGlobal = hGlobal;
			pmedium->tymed = TYMED_HGLOBAL;
			pmedium->pUnkForRelease = nullptr;

			return S_OK;
			
		}
		return DV_E_FORMATETC;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::GetDataHere(FORMATETC* pFormatetc, STGMEDIUM* pmedium)
	{

		MNEMOSY_TRACE("IDataObject::GetDataHere: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::QueryGetData(FORMATETC* pFormatetc) {

		if (pFormatetc->cfFormat == CF_HDROP && pFormatetc->tymed == TYMED_HGLOBAL) {
			MNEMOSY_TRACE("IDataObject::QueryGetData: Called: S_OK");
			return S_OK;
		}
		MNEMOSY_TRACE("IDataObject::QueryGetData: Called: DV_E_FORMATETC");
		return DV_E_FORMATETC;

	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::GetCanonicalFormatEtc(FORMATETC* pFormatetcIn, FORMATETC* pFormatetcOut) {
		MNEMOSY_TRACE("IDataObject::GetCanonicalFormatEtc: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::SetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium, BOOL fRelease)
	{


		if (pFormatetc->cfFormat == CF_HDROP && pFormatetc->tymed == TYMED_HGLOBAL) {

			// Prepare file drop data

			std::vector<wchar_t> buffer;
			for (const auto& filePath : m_filePaths) {

				buffer.insert(buffer.end(), filePath.begin(), filePath.end());
				buffer.push_back(L'\0');

			}
			buffer.push_back(L'\0');// Double null-terminated

			//
			//// Allocate memory for file drop structure
			HGLOBAL hGlobal = GlobalAlloc(GHND, sizeof(DROPFILES) + (buffer.size() * sizeof(wchar_t)));


			//
			DROPFILES* pDropFiles = reinterpret_cast<DROPFILES*>(GlobalLock(hGlobal));
			pDropFiles->pFiles = sizeof(DROPFILES);
			pDropFiles->fWide = TRUE; // Indicate Unicode file paths
			memcpy((char*)pDropFiles + sizeof(DROPFILES), buffer.data(), buffer.size() * sizeof(wchar_t));

			GlobalUnlock(hGlobal);


			//// Set data medium
			pmedium->hGlobal = hGlobal;
			pmedium->tymed = TYMED_HGLOBAL;
			pmedium->pUnkForRelease = nullptr;

			return S_OK;
		}
		return DV_E_FORMATETC;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc) {
		MNEMOSY_TRACE("IDataObject::EnumFormatEtc: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::DAdvise(FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) {
		MNEMOSY_TRACE("IDataObject::DAdvice: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::DUnadvise(DWORD dwConnection) {
		MNEMOSY_TRACE("IDataObject::DAdvice: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise) {
		MNEMOSY_TRACE("IDataObject::EnumDAdvice: NotImpl");
		return OLE_E_ADVISENOTSUPPORTED;
	}

	
	// ======== DROP SOURCE 

	STDMETHODIMP_(HRESULT __stdcall) DropS::QueryInterface(REFIID riid, void** ppvObj)
	{
		if (!ppvObj) {
			MNEMOSY_TRACE("DropSource::QuearyInterface: E_INVALIDARG");
			return E_INVALIDARG;
		}

		*ppvObj = NULL;
		if (riid == IID_IUnknown || riid == CLSID_DropSource || riid == IID_IDropSource)
		{
			// Increment the reference count and return the pointer.
			*ppvObj = (LPVOID)this;
			AddRef();
			MNEMOSY_TRACE("DropSource::QuearyInterface: NOERROR");
			return NOERROR;

		}

		MNEMOSY_TRACE("DropSource::QuearyInterface: E_NOINTERFACE");
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG __stdcall) DropS::AddRef(){

		InterlockedIncrement(&m_cRef);
		return m_cRef;
	}

	STDMETHODIMP_(ULONG __stdcall) DropS::Release() {

		ULONG ulRefCount = InterlockedDecrement(&m_cRef);
		if (0 == m_cRef) {
			delete this;
		}
		return ulRefCount;
	}

	HRESULT __stdcall DropS::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) {

		// couldnt test yet if this is working

		bool haveMouseButton = (grfKeyState && MK_LBUTTON) || (grfKeyState && MK_RBUTTON);

		//MNEMOSY_TRACE("DropS::QueryContinueDrag: haveMouseButton = {}", haveMouseButton);

		if (fEscapePressed && haveMouseButton) {
			return DRAGDROP_S_CANCEL;
		}
		else if (!haveMouseButton) {
			return DRAGDROP_S_DROP;
		}
				
		return S_OK;
	}

	HRESULT __stdcall DropS::GiveFeedback(DWORD dwEffect) {
		// Give Feedback allows the implementor to set the cusror 
		// or implement some other visual effect so the user is aware of what is happening 
		//MNEMOSY_TRACE("DropS::GiveFeedback: Called return DRAGDROP_S_USEDEFAULTCURSORS");
		return DRAGDROP_S_USEDEFAULTCURSORS;

		//return E_NOTIMPL;
	}

	// IClassFactories


	HRESULT __stdcall DropSourceClassFactory::QueryInterface(REFIID riid, void** ppvObj) {

		if (riid == IID_IUnknown || riid == IID_IClassFactory) {

			MNEMOSY_TRACE("DropSourceClassFactory::QueryInterface: NOERROR");
			*ppvObj = this;
			AddRef();
			return NOERROR;
		}

		*ppvObj = 0;
		MNEMOSY_TRACE("DropSourceClassFactory::QueryInterface: E_NOINTERFACE");
		return E_NOINTERFACE;
	}

	HRESULT __stdcall DropSourceClassFactory::CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj)
	{
		MNEMOSY_TRACE("DropSourceClassFactory::CreateInstance");


		HRESULT hr;
		// Assume an error by clearing callers handle
		*ppvObj = 0;
		if (punkOuter) {
			return CLASS_E_NOAGGREGATION;
		}
		else {
			IDropSource* obj = new DropS();

			//*ppvObj = obj;

			hr = obj->QueryInterface(riid, ppvObj);

			obj->Release();

			return hr;
		}

		return E_NOINTERFACE;
	}

	HRESULT __stdcall DataObjectClassFactory::CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj)
	{
		MNEMOSY_TRACE("DataObjectClassFactory::CreateInstance");

		HRESULT hr;
		// Assume an error by clearing callers handle
		*ppvObj = 0;
		if (punkOuter) {
			return CLASS_E_NOAGGREGATION;
		}
		else {
			IDataObject* obj = new FileDataObject();

			hr = obj->QueryInterface(riid, ppvObj);

			obj->Release();


			return hr;
		}

		return E_NOINTERFACE;
	}

	// utilities
	void CheckComError(const char* classAndfunctionName, HRESULT hr) {
		
		if (FAILED(hr)) {

			_com_error comError(hr);
			LPCTSTR errorMsg = comError.ErrorMessage();
			MNEMOSY_WARN("ComError: {} Failed: {}", classAndfunctionName, std::string(errorMsg));
		}
	}

} // mnemosy::core

#endif // !MNEMOSY_PLATFORM_WINDOW