#include "Include/Core/Utils/DropHandler_Windows.h"

#include "Include/MnemosyConfig.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Window.h"
#include "Include/Core/Log.h"
#include "Include/Core/Utils/DropManager_Windows.h"


#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include <filesystem>

#ifdef MNEMOSY_PLATFORM_WINDOWS

// windows includes
#include <shlobj_core.h>
#include <comdef.h>
#include <TCHAR.H>

//#include <shobjidl_core.h>
//#include <Shellapi.h>
//#include <objidl.h>
//#include <oleidl.h>


namespace mnemosy::core {
	
	//==== public methods
	// {5D23E357-A104-413F-A7FE-31EC9713C2B1}

	DropHandler::DropHandler() {


		Initialize();
		RegisterDropTarget();	
	}

	DropHandler::~DropHandler() {
		
	}

	// maybe drop DropHandler can keep drop manager pointer istead of engine
	void DropHandler::RegisterDropTarget() {

		HRESULT hr;

		//m_pDropManagerFactory = new DropManagerClassFactory();
		
		//DWORD class_reg;
		//hr = CoRegisterClassObject(CLSID_DropManager, (IUnknown*)m_pDropManagerFactory, CLSCTX_INPROC_SERVER, REGCLS_SINGLEUSE, &class_reg);
		//Check_HRESULT_Error("CoRegisterClassObject DropManager", hr);

		//hr = CoCreateInstance(CLSID_DropManager, NULL, CLSCTX_INPROC_SERVER, IID_IDropTarget, reinterpret_cast<void**>(&m_pDropManager));
		//Check_HRESULT_Error("CoCreateInstance for DropManager", hr);

		m_pDropManager = new DropManager();

		hr = RegisterDragDrop(glfwGetWin32Window(&MnemosyEngine::GetInstance().GetWindow().GetWindow()), static_cast<IDropTarget*>(m_pDropManager));
		Check_HRESULT_Error("DropHandler::RegisterDropTarget: ", hr);

	}

	void DropHandler::Initialize() {

		HRESULT hr;
		{
			hr = CoInitialize(NULL); // we probably dont need this
			Check_HRESULT_Error("DropHandler::Initialize::CoInitialize() ", hr);

			hr = OleInitialize(NULL);
			Check_HRESULT_Error("DropHandler::Initialize::OleInitialize() ", hr);
		}
		

		{
			m_pDropSourceFactory	= new DropSourceClassFactory();
			m_pDataObjectFactory	= new DataObjectClassFactory();
			m_pEnumFormatEtcFactory = new EnumFormatEtcClassFactory();

			DWORD class_reg2;
			hr = CoRegisterClassObject(CLSID_DropSource, (IUnknown*)m_pDropSourceFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &class_reg2);
			Check_HRESULT_Error("CoRegisterClassObject DropSource", hr);
			
			
			DWORD class_reg;
			hr = CoRegisterClassObject(CLSID_FileDataObject, (IUnknown*)m_pDataObjectFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &class_reg);
			Check_HRESULT_Error("CoRegisterClassObject FileDataObject", hr);


			DWORD class_reg3;
			hr = CoRegisterClassObject(CLSID_DataFormatEtc, (IUnknown*)m_pEnumFormatEtcFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &class_reg3);
			Check_HRESULT_Error("CoRegisterClassObject DataEnumFormatEtc", hr);
		}


	}

	void DropHandler::Uninitialize() {
		


		CoUninitialize(); // we may dont need this at all
		OleUninitialize();

		if(m_pDropSourceFactory)
			delete m_pDropSourceFactory;
		if(m_pDataObjectFactory)
			delete m_pDataObjectFactory;
		if(m_pEnumFormatEtcFactory)
			delete m_pEnumFormatEtcFactory;

		m_pDropSourceFactory = nullptr;
		m_pDataObjectFactory = nullptr;
		m_pEnumFormatEtcFactory = nullptr;

		// I think the reason this crashes is because glfw already calls this function when we close the window..
		//HRESULT hr = RevokeDragDrop(glfwGetWin32Window(&MnemosyEngine::GetInstance().GetWindow().GetWindow()));
		//Check_HRESULT_Error("DropHandler::Uninitialze:RevokeDragDrop: ", hr);

		if (m_pDropManager) {
			delete m_pDropManager;
			m_pDropManager = nullptr;
		}
	}

	void DropHandler::BeginDrag() {
		

		//check out this maybe https://stackoverflow.com/questions/449872/how-drag-and-drop-of-files-is-done
		MNEMOSY_DEBUG("DropHandler::BeginDrag:");

		// TODO Clear this
		std::vector<std::string> filesToDrag;

		std::filesystem::path p1 = std::filesystem::path(R"(D:/Drop1.png)");
		std::filesystem::path p2 = std::filesystem::path(R"(D:/Drop2.png)");

		filesToDrag.push_back(p1.generic_string());
		filesToDrag.push_back(p2.generic_string());



		FORMATETC* etc = new FORMATETC();// = { CF_HDROP, NULL, DVASPECT_DOCPRINT, -1, TYMED_HGLOBAL };
		etc->cfFormat = CF_HDROP;
		etc->dwAspect = DVASPECT_CONTENT;
		etc->tymed = TYMED_HGLOBAL;
		etc->ptd = NULL;
		etc->lindex = -1;

		STGMEDIUM* stg = new STGMEDIUM();

		HRESULT hr;

		DropSource* pDropSource = new DropSource();
		//hr = CoCreateInstance(CLSID_DropSource, NULL, CLSCTX_INPROC_SERVER, IID_IDropSource, reinterpret_cast<void**>(&pDropSource));
		//Check_HRESULT_Error("CoCreateInstance for DropSource", hr);

		// Create data object
		FileDataObject* pDataObject = new FileDataObject(etc,stg);
		//hr = CoCreateInstance(CLSID_FileDataObject, NULL, CLSCTX_INPROC_SERVER, IID_IDataObject, reinterpret_cast<void**>(&pDataObject));
		//Check_HRESULT_Error("CoCreateInstance for FileDataObject", hr);


		pDataObject->Init(filesToDrag);

		//hr = pDataObject->GetData(etc, stg);
		//Check_HRESULT_Error("FileDataObject::GetData:", hr);



		// // Start drag operation
		DWORD dwEffect = 0;

		hr = DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);
		if (SUCCEEDED(hr)) {
			MNEMOSY_INFO("DropHandler::BeginDrag:DoDragDrop() Succeeded");
		}
		else {
			Check_HRESULT_Error("DoDragDrop():", hr);
		}

		pDataObject->Release();
		pDropSource->Release();
	}

	void DropHandler::SetDropTargetActive(bool state) {

		m_pDropManager->SetDropActive(state);
	}

	// ========= Interface Implementations ====================================
	// ========================================================================

	// ==== DropSource 
	
	DropSource::DropSource() {
		//MNEMOSY_TRACE("DropSource::Constructor:");
	}

	DropSource::~DropSource() {
		//MNEMOSY_TRACE("DropSource::Destructor:");
	}

	STDMETHODIMP_(HRESULT __stdcall) DropSource::QueryInterface(REFIID riid, void** ppvObj)
	{
		if (!ppvObj) {
			//MNEMOSY_TRACE("DropSource::QuearyInterface: E_INVALIDARG");
			return E_INVALIDARG;
		}

		*ppvObj = NULL;
		if (riid == IID_IUnknown || riid == CLSID_DropSource || riid == IID_IDropSource)
		{
			// Increment the reference count and return the pointer.
			AddRef();
			*ppvObj = reinterpret_cast<void*>(this); //(LPVOID)this;
			//MNEMOSY_TRACE("DropSource::QuearyInterface: NOERROR");
			return S_OK;
		}

		//MNEMOSY_TRACE("DropSource::QuearyInterface: E_NOINTERFACE");
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG __stdcall) DropSource::AddRef() {

		ULONG ulRefCount = InterlockedIncrement(&m_cRef);
		//MNEMOSY_TRACE("DropSource::AddRef: Count: {}", (int)m_cRef);
		return ulRefCount;
	}

	STDMETHODIMP_(ULONG __stdcall) DropSource::Release() {

		ULONG ulRefCount = InterlockedDecrement(&m_cRef);
		//MNEMOSY_TRACE("DropSource::Release: Count: {}", (int)m_cRef);
		if (0 == m_cRef) {
			delete this;
		}
		return ulRefCount;
	}

	HRESULT __stdcall DropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) {

		bool haveMouseButton = (grfKeyState && MK_LBUTTON) || (grfKeyState && MK_RBUTTON);

		if (fEscapePressed && haveMouseButton) {
			return DRAGDROP_S_CANCEL;
		}
		else if (!haveMouseButton) {
			return DRAGDROP_S_DROP;
		}
		return S_OK;
	}

	HRESULT __stdcall DropSource::GiveFeedback(DWORD dwEffect) {
		// Give Feedback allows the implementor to set the cusror 
		// or implement some other visual effect so the user is aware of what is happening 
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}
	
	// ==== FileDataObject

	void FileDataObject::Init(const std::vector<std::string>& filePaths) {

		m_filePaths = filePaths;
	}

	bool FileDataObject::LookupFormatEtc(FORMATETC* pfmtetc) {

		
		if (pfmtetc->cfFormat == m_pfmtetc->cfFormat &&
			pfmtetc->tymed == m_pfmtetc->tymed) {

			MNEMOSY_TRACE("IDataObject::LookupFormat: Match");
			return true;
		}

		return false;
	}

	FileDataObject::FileDataObject(FORMATETC* fmtetc, STGMEDIUM* stgmed) {

		m_nNumFormats = 1;
		m_pfmtetc = fmtetc;
		m_pstgmed = stgmed;

		MNEMOSY_TRACE("IDataObject::Constructor:")
	}

	FileDataObject::~FileDataObject() {

		if (m_pfmtetc) {
			delete m_pfmtetc;
			m_pfmtetc = nullptr;
		}

		if (m_pstgmed) {
			delete m_pstgmed;
			m_pstgmed = nullptr;
		}

		MNEMOSY_TRACE("IDataObject::Destructor:")
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::QueryInterface(REFIID riid, void** ppvObj) {

		//MNEMOSY_TRACE("IDataObject::QuearyInterface:");
		if (!ppvObj) {
			//MNEMOSY_TRACE("IDataObject::QuearyInterface: E_INVALIDARG");
			return E_INVALIDARG;
		}

		*ppvObj = NULL;
		if (riid == IID_IUnknown || riid == CLSID_FileDataObject || riid == IID_IDataObject)
		{
			// Increment the reference count and return the pointer.
			AddRef();
			*ppvObj = reinterpret_cast<void*>(this);// (LPVOID)this;
			MNEMOSY_TRACE("IDataObject::QuearyInterface: NOERROR");
			return S_OK;
		}
		MNEMOSY_TRACE("IDataObject::QuearyInterface:E_NOINTERFACE");
		
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG __stdcall) FileDataObject::AddRef() {

		InterlockedIncrement(&m_cRef);
		MNEMOSY_TRACE("IDataObject::AddRef: Count: {}", (int)m_cRef);
		return m_cRef;
	}

	STDMETHODIMP_(ULONG __stdcall) FileDataObject::Release() {
		// Decrement the object's internal counter.
		ULONG ulRefCount = InterlockedDecrement(&m_cRef);
		MNEMOSY_TRACE("IDataObject::Release: Count: {}",int(m_cRef));

		if (m_cRef == 0) {
			delete this;
		}
		return ulRefCount;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::GetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium) {	

		if (pFormatetc->cfFormat == CF_HDROP && pFormatetc->tymed == TYMED_HGLOBAL) {
			// Convert file paths from multibyte to wide-character strings
			std::vector<std::wstring> unicodeFilePaths;
			for (const auto& filePath : m_filePaths) {
				int wcharCount = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, nullptr, 0);
				if (wcharCount > 0) {
					std::wstring unicodeFilePath;
					unicodeFilePath.resize(wcharCount - 1); // Exclude null terminator
					MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, &unicodeFilePath[0], wcharCount);
					unicodeFilePaths.push_back(std::move(unicodeFilePath));
				}
			}

			// Calculate total size needed for DROPFILES structure
			UINT uBuffSize = sizeof(DROPFILES);
			for (const auto& filePath : unicodeFilePaths) {
				uBuffSize += (filePath.size() + 1) * sizeof(wchar_t); // Include null terminator
			}

			// Allocate memory from the heap for the DROPFILES struct and file paths
			HGLOBAL hGlobal = GlobalAlloc(GHND, uBuffSize);
			if (hGlobal != nullptr) {
				DROPFILES* pDrop = reinterpret_cast<DROPFILES*>(GlobalLock(hGlobal));
				if (pDrop != nullptr) {
					// Fill in the DROPFILES struct
					pDrop->pFiles = sizeof(DROPFILES);
					pDrop->fWide = TRUE;

					// Copy file paths into memory after the end of the DROPFILES struct
					wchar_t* pszBuff = reinterpret_cast<wchar_t*>(reinterpret_cast<LPBYTE>(pDrop) + sizeof(DROPFILES));
					for (const auto& filePath : unicodeFilePaths) {
						wcscpy_s(pszBuff, filePath.size() + 1, filePath.c_str());
						pszBuff += filePath.size() + 1; // Move to the next position after null terminator
					}

					GlobalUnlock(hGlobal);

					// Set data medium
					pmedium->hGlobal = hGlobal;
					pmedium->tymed = TYMED_HGLOBAL;
					pmedium->pUnkForRelease = nullptr;
					return S_OK;
				}
				GlobalFree(hGlobal); // Release memory if locking fails
			}
			return E_OUTOFMEMORY; // Memory allocation failed
		}
		return DV_E_FORMATETC; // We don't support other formats
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::GetDataHere(FORMATETC* pFormatetc, STGMEDIUM* pmedium) {

		MNEMOSY_DEBUG("IDataObject::GetDataHere: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::QueryGetData(FORMATETC* pFormatetc) {



		if (LookupFormatEtc(pFormatetc)) {

			MNEMOSY_TRACE("IDataObject::QueryGetData: S_OK");
			return S_OK;
		}

		MNEMOSY_TRACE("IDataObject::QueryGetData: DV_E_FORMATETC");
		return DV_E_FORMATETC;

		//return S_OK;
		if (pFormatetc->cfFormat == CF_HDROP) {

			//MNEMOSY_DEBUG("FileDataObject::QueryGetData: CF_HDROP ACCEPT");
			//pFormatetc->dwAspect = DVASPECT_DOCPRINT;
		}

		if (pFormatetc->cfFormat == CF_HDROP && pFormatetc->tymed == TYMED_HGLOBAL) {

			MNEMOSY_TRACE("IDataObject::QueryGetData: S_OK");
			//pFormatetc->dwAspect = DVASPECT_CONTENT;
			return S_OK;
		}

		MNEMOSY_TRACE("IDataObject::QueryGetData: DV_E_FORMATETC");
		return DV_E_FORMATETC;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::GetCanonicalFormatEtc(FORMATETC* pFormatetcIn, FORMATETC* pFormatetcOut) {
		//MNEMOSY_TRACE("IDataObject::GetCanonicalFormatEtc: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::SetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium, BOOL fRelease)
	{
		
		//MNEMOSY_DEBUG("FileDataObject::SetData:");
		return E_NOTIMPL;		

		if (pFormatetc->cfFormat == CF_HDROP && pFormatetc->tymed == TYMED_HGLOBAL) {
		

			//MNEMOSY_TRACE("IDataObject::GetData: S_OK");
			//return S_OK;
			
			
			// Handle CF_HDROP format data
			// Extract file paths from pMedium->hGlobal
			// Store the file paths internally for later retrieval in GetData
			// If fRelease is TRUE, release the medium
			if (fRelease)
				ReleaseStgMedium(pmedium);

			//MNEMOSY_TRACE("IDataObject::SetData: S_OK");
			return S_OK;
		}

		//MNEMOSY_TRACE("IDataObject::SetData: NotImpl");
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc) {

		

			//return S_OK;
		if (dwDirection == DATADIR_GET) {
			
			MNEMOSY_DEBUG("IDataObject::EnumFormatEtc: CreateInstance of EnumFormatEtc");
			DataEnumFormatEtc* formatic = new DataEnumFormatEtc();
			//HRESULT hr = CoCreateInstance(CLSID_DataFormatEtc, NULL, CLSCTX_INPROC_SERVER, IID_IEnumFORMATETC, reinterpret_cast<void**>(&formatic));
			//HRESULT hr = CoCreateInstance(CLSID_DataFormatEtc, NULL, CLSCTX_INPROC_SERVER, IID_IEnumFORMATETC, reinterpret_cast<void**>(&formatic));
			//Check_HRESULT_Error("IDataObject::EnumFormatEtc::CoCreateInstance for DataFormatEtc", hr);
			
			*ppenumFormatetc = formatic;
			return S_OK;

		}

		MNEMOSY_TRACE("IDataObject::EnumFormatEtc: E_NOTIMPL");
		*ppenumFormatetc = NULL;
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::DAdvise(FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) {
		//MNEMOSY_TRACE("IDataObject::DAdvice: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::DUnadvise(DWORD dwConnection) {
		//MNEMOSY_TRACE("IDataObject::DAdvice: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise) {
		//MNEMOSY_TRACE("IDataObject::EnumDAdvice: NotImpl");
		return OLE_E_ADVISENOTSUPPORTED;
	}


	// ==== DataEnumFormatEtc
	
	DataEnumFormatEtc::DataEnumFormatEtc( )
	{
		m_nIndex = 0;
		m_nNumFormats = 1; // hardcoded for now
		m_pFormatetc = new FORMATETC();
		m_pFormatetc->cfFormat = CF_HDROP;
		m_pFormatetc->dwAspect = DVASPECT_CONTENT;
		m_pFormatetc->ptd = NULL;
		m_pFormatetc->tymed = TYMED_HGLOBAL;
		m_pFormatetc->lindex = -1;
	}

	DataEnumFormatEtc::~DataEnumFormatEtc()
	{
		MNEMOSY_TRACE("DataEnumFormatEtc::Release: Destructor");
		if (m_pFormatetc) {
			//CoTaskMemFree(m_pFormatetc);
			delete m_pFormatetc;
			m_pFormatetc = nullptr;
		}
	}

	HRESULT __stdcall DataEnumFormatEtc::QueryInterface(REFIID riid, void** ppvObj) {
		if (!ppvObj) {
			//MNEMOSY_TRACE("DataEnumFormatEtc::QuearyInterface: E_INVALIDARG");
			return E_INVALIDARG;
		}
		if (riid == IID_IUnknown || riid == IID_IEnumFORMATETC || riid == CLSID_DataFormatEtc)
		{
			// Increment the reference count and return the pointer.
			*ppvObj =  reinterpret_cast<void*>(this); //(LPVOID)this; 
			AddRef();
			MNEMOSY_TRACE("DataEnumFormatEtc::QueryInterface: S_OK");
			return S_OK;

		}

		MNEMOSY_TRACE("DataEnumFormatEtc::QueryInterface: E_NOINTERFACE");
		*ppvObj = NULL;

		return E_NOINTERFACE;
	}

	ULONG __stdcall DataEnumFormatEtc::AddRef() {
		InterlockedIncrement(&m_cRef);
		return m_cRef;
	}

	ULONG __stdcall DataEnumFormatEtc::Release() {

		ULONG ulRefCount = InterlockedDecrement(&m_cRef);
		MNEMOSY_TRACE("DataEnumFormatEtc::Release: RefCount: {}",int(m_cRef));

		if (0 == m_cRef) {
			delete this;
		}
		return ulRefCount;
	}

	HRESULT __stdcall DataEnumFormatEtc::Clone(IEnumFORMATETC** ppenum) {

		DataEnumFormatEtc* formatic;

		MNEMOSY_DEBUG("DataEnumFormatEtc::Clone: CreateInstance of EnumFormatEtc");
		HRESULT hr = CoCreateInstance(CLSID_DataFormatEtc, NULL, CLSCTX_INPROC_SERVER, IID_IEnumFORMATETC, reinterpret_cast<void**>(&formatic));
		Check_HRESULT_Error("DataEnumFormatEtc::CoCreateInstance: ",hr);

		formatic->m_fIndex = m_nIndex;
		//formatic->m_fIndex = m_fIndex;
		*ppenum = formatic;
		//formatic->Release();
		

		//MNEMOSY_TRACE("DataEnumFormatEtc::Clone: S_OK");
		return S_OK;
	}

	HRESULT __stdcall DataEnumFormatEtc::Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched) {


		if (celt == 0 || rgelt == 0) {
			return E_INVALIDARG;
		}

		int copied = 0;
		while (m_nIndex < m_nNumFormats && copied < celt) {
			DeepCopyFormatEtx(rgelt,m_pFormatetc);
			copied++;
			m_nIndex++;
		}

		MNEMOSY_TRACE("DataEnumFormatEtc::Next: Copied: {} ", copied);

		if (pceltFetched == NULL) {
			pceltFetched = new ULONG(copied);
		}

		if (copied == celt) {
			//MNEMOSY_TRACE("DataEnumFormatEtc::Next: S_OK");
			return S_OK;
		}


		return S_FALSE;
		//////
		// this function looks fuckting scary man
		MNEMOSY_TRACE("DataEnumFormatEtc::Next:");
		int cc = 0; // copiedCount
		FORMATETC* pf = nullptr;
		pf = rgelt;

		FORMATETC F;
		F.ptd = NULL;
		F.dwAspect = DVASPECT_CONTENT;
		F.lindex = -1;
		// maybe they should not be here
		F.tymed = TYMED_HGLOBAL;
		F.cfFormat = CF_HDROP;




		while (m_fIndex < 2 && celt>0) {
			cc++;

			if (m_fIndex == 0) {
				MNEMOSY_TRACE("DataEnumFormatEtc::Next: index is 0");
				F.tymed = TYMED_HGLOBAL;
				F.cfFormat = CF_HDROP;
			}
			else if (m_fIndex == 1) {
				MNEMOSY_WARN("DataEnumFormatEtc::Next: index is 1");
				//return E_NOTIMPL;
			}
			// Move into place.
			memmove(pf, &F, sizeof(FORMATETC));

			// Prepare for next iteration.
			celt--;
			m_fIndex++;
			pf = pf + sizeof(FORMATETC);

		}

		if (pceltFetched == NULL) {
			pceltFetched = new ULONG(cc);
		}

		if (cc > 0) {
			MNEMOSY_TRACE("DataEnumFormatEtc::Next: S_OK");
			return S_OK;
		}



		return E_NOTIMPL;
	}

	HRESULT __stdcall DataEnumFormatEtc::Reset() {

		MNEMOSY_TRACE("DataEnumFormatEtc::Reset:");
		m_nIndex = 0;
		m_fIndex = 0;
		return S_OK;
	}

	HRESULT __stdcall DataEnumFormatEtc::Skip(ULONG celt) {


		m_nIndex += celt;
		if (m_nIndex <= m_nNumFormats) {
			//MNEMOSY_TRACE("DataEnumFormatEtc::Skip: S_OK");
			return S_OK;
		}
			//MNEMOSY_TRACE("DataEnumFormatEtc::Skip: S_FALSE");
		return S_FALSE;
	}

	HRESULT DataEnumFormatEtc::DeepCopyFormatEtx(FORMATETC* dest, FORMATETC* source) {

		dest = source;

		if (source->ptd) {
			dest->ptd = reinterpret_cast<DVTARGETDEVICE*>(CoTaskMemAlloc(sizeof(DVTARGETDEVICE))); //CoTaskMemAlloc(sizeof(DVTARGETDEVICE));
			dest->ptd = source->ptd;
		}				

		return S_OK;
		//return E_NOTIMPL;
	}


	// ========= Com Class Factories =========================================
	// =======================================================================
	
	// == DropSourceFactory
	HRESULT __stdcall DropSourceClassFactory::QueryInterface(REFIID riid, void** ppvObj) {

		if (riid == IID_IUnknown || riid == IID_IClassFactory) {

			*ppvObj = this;
			AddRef();
			//MNEMOSY_TRACE("DropSourceClassFactory::QueryInterface: NOERROR");
			return NOERROR;
		}

		*ppvObj = 0;
		//MNEMOSY_TRACE("DropSourceClassFactory::QueryInterface: E_NOINTERFACE");
		return E_NOINTERFACE;
	}

	HRESULT __stdcall DropSourceClassFactory::CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj) {

		// Assume an error by clearing callers handle
		*ppvObj = 0;
		if (punkOuter) {
			return CLASS_E_NOAGGREGATION;
		}
		else {
			DropSource* obj = new DropSource();

			HRESULT hr = obj->QueryInterface(riid, ppvObj);
			obj->Release();

			//MNEMOSY_TRACE("DropSourceClassFactory::CreateInstance: ");
			return hr;
		}

		//MNEMOSY_TRACE("DropSourceClassFactory::CreateInstance: E_NOINTERFACE");
		return E_NOINTERFACE;
	}

	// == DataObject Factory
	HRESULT __stdcall DataObjectClassFactory::QueryInterface(REFIID riid, void** ppvObj) {


		if (riid == IID_IUnknown || riid == IID_IClassFactory) {

			*ppvObj = this;
			AddRef();
			//MNEMOSY_TRACE("DropSourceClassFactory::QueryInterface: NOERROR");
			return NOERROR;
		}

		*ppvObj = 0;
		//MNEMOSY_TRACE("DropSourceClassFactory::QueryInterface: E_NOINTERFACE");
		return E_NOINTERFACE;


		if (riid != IID_IUnknown && riid != IID_IClassFactory) {
			*ppvObj = 0;
			//MNEMOSY_TRACE("DataObjectClassFactory::QueryInterface: E-NOINTERFACE");
			return E_NOINTERFACE;
		}

		*ppvObj = this;
		AddRef();
		//MNEMOSY_TRACE("DataObjectClassFactory::QueryInterface: NOERROR");
		return NOERROR;
	}

	HRESULT __stdcall DataObjectClassFactory::CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj) {

		// Assume an error by clearing callers handle
		*ppvObj = 0;
		if (punkOuter) {
			return CLASS_E_NOAGGREGATION;
		}
		else {



			//FileDataObject* obj = new FileDataObject();
			//HRESULT hr = obj->QueryInterface(riid, ppvObj);
			//obj->Release();
			//MNEMOSY_TRACE("DataObjectClassFactory::CreateInstance");
			//return hr;
		}

		//MNEMOSY_TRACE("DataObjectClassFactory::CreateInstance. E_NOINTERFACE");
		return E_NOINTERFACE;
	}
	
	// == EnumFormatEtc Factory
	HRESULT __stdcall EnumFormatEtcClassFactory::QueryInterface(REFIID riid, void** ppvObj) {

		if (riid == IID_IUnknown || riid == IID_IClassFactory) {
			*ppvObj = this;
			AddRef();
			//MNEMOSY_TRACE("EnumFormatEtcClassFactory::QueryInterface: NOERROR");
			return NOERROR;
		}

		*ppvObj = 0;
		//MNEMOSY_TRACE("EnumFormatEtcClassFactory::QueryInterface: E_NOINTERFACE");
		return E_NOINTERFACE;
	}

	HRESULT __stdcall EnumFormatEtcClassFactory::CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj) {

		HRESULT hr;
		// Assume an error by clearing callers handle
		*ppvObj = 0;
		if (punkOuter) {
			return CLASS_E_NOAGGREGATION;
		}
		else {


			DataEnumFormatEtc* obj = new DataEnumFormatEtc();

			hr = obj->QueryInterface(riid, ppvObj);
			obj->Release();
			//MNEMOSY_TRACE("EnumFormatEtcClassFactory::CreateInstance:");
			return hr;
		}

		//MNEMOSY_TRACE("EnumFormatEtcClassFactory::CreateInstance: E_NOINTERFACE");
		return E_NOINTERFACE;
	}


	// ========= Utility Methods =============================================
	// =======================================================================
	void Check_HRESULT_Error(const char* prefixMessage, HRESULT hr) {

		if (FAILED(hr)) {

			_com_error comError(hr);
			LPCTSTR errorMsg = comError.ErrorMessage();
			MNEMOSY_WARN("ComError: {} Failed: {}", prefixMessage, std::string(errorMsg));
		}
	}

	


}// !mnemosy::core
	//==== private methods

#endif // !MNEMOSY_PLATFORM_WINDOW