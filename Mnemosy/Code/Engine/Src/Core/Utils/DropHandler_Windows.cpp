#include "Include/Core/Utils/DropHandler_Windows.h"

#include "Include/MnemosyConfig.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Window.h"
#include "Include/Core/Log.h"
#include "Include/Core/Utils/DropManager_Windows.h"

#include <filesystem>

#ifdef MNEMOSY_PLATFORM_WINDOWS

// windows includes
#include <shlobj_core.h>
#include <comdef.h>
#include <TCHAR.H>
// not needed here for now
//#include <shobjidl_core.h>
//#include <Shellapi.h>
//#include <objidl.h>
//#include <oleidl.h>


namespace mnemosy::core {
	
	//==== public methods

	void DropHandler::Init(GLFWwindow& window) {
		
		// Dont need this if we initialize ole
		//hr = CoInitialize(NULL); 
		//Check_HRESULT_Error("DropHandler::Initialize::CoInitialize() ", hr);

		HRESULT hr = OleInitialize(NULL);
		Check_HRESULT_Error("DropHandler::Initialize::OleInitialize(): ", hr);
		
		m_pDropManager = new DropManager();
		hr = RegisterDragDrop(glfwGetWin32Window(&window), static_cast<IDropTarget*>(m_pDropManager));
		Check_HRESULT_Error("DropHandler::Initialize::RegisterDragDrop(): ", hr);
		
		
		// Apperently I don't need the factory stuff at all..
		
		m_pDropSourceFactory	= new DropSourceClassFactory();
		m_pDataObjectFactory	= new DataObjectClassFactory();
		m_pEnumFormatEtcFactory = new EnumFormatEtcClassFactory();

		DWORD class_reg2;
		hr = CoRegisterClassObject(CLSID_DropSource, (IUnknown*)m_pDropSourceFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &class_reg2);
		Check_HRESULT_Error("DropHandler::Initialize::CoRegisterClassObject(): DropSource ", hr);
			
			
		DWORD class_reg;
		hr = CoRegisterClassObject(CLSID_FileDataObject, (IUnknown*)m_pDataObjectFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &class_reg);
		Check_HRESULT_Error("DropHandler::Initialize::CoRegisterClassObject(): FileDataObject ", hr);


		DWORD class_reg3;
		hr = CoRegisterClassObject(CLSID_DataFormatEtc, (IUnknown*)m_pEnumFormatEtcFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &class_reg3);
		Check_HRESULT_Error("DropHandler::Initialize::CoRegisterClassObject(): DataEnumFormatEtc ", hr);
		
	}

	void DropHandler::Shutdown() {
		
		// I think the reason this crashes is because glfw already calls these functions when terminating the window..
		// OleUninitialize();
		//HRESULT hr = RevokeDragDrop(glfwGetWin32Window(&MnemosyEngine::GetInstance().GetWindow().GetWindow()));
		//Check_HRESULT_Error("DropHandler::Uninitialze:RevokeDragDrop: ", hr);
		
		if(m_pDropSourceFactory)
			delete m_pDropSourceFactory;
		if(m_pDataObjectFactory)
			delete m_pDataObjectFactory;
		if(m_pEnumFormatEtcFactory)
			delete m_pEnumFormatEtcFactory;

		m_pDropSourceFactory = nullptr;
		m_pDataObjectFactory = nullptr;
		m_pEnumFormatEtcFactory = nullptr;		


		if (m_pDropManager) {
			delete m_pDropManager;
			m_pDropManager = nullptr;
		}
	}

	void DropHandler::BeginDrag(std::vector<std::string>& filesToDrag) {


		MNEMOSY_TRACE("DropHandler::BeginDrag:");

		FORMATETC* etc = new FORMATETC();
		etc->cfFormat = CF_HDROP;
		etc->dwAspect = DVASPECT_CONTENT;
		etc->tymed = TYMED_HGLOBAL;
		etc->ptd = NULL;
		etc->lindex = -1;

		STGMEDIUM* stg = new STGMEDIUM();
				

		// It seems I dont need to use the factories and could just new DropSource()
		// but its probably good to register with windows
		DropSource* pDropSource;
		HRESULT hr = CoCreateInstance(CLSID_DropSource, NULL, CLSCTX_INPROC_SERVER, IID_IDropSource, reinterpret_cast<void**>(&pDropSource));
		Check_HRESULT_Error("CoCreateInstance for DropSource", hr);

		FileDataObject* pDataObject;
		hr = CoCreateInstance(CLSID_FileDataObject, NULL, CLSCTX_INPROC_SERVER, IID_IDataObject, reinterpret_cast<void**>(&pDataObject));
		Check_HRESULT_Error("CoCreateInstance for FileDataObject", hr);
				
		// just pass the vector of string filepaths to the FileDataObject
		pDataObject->Init(filesToDrag);

		// Start drag operation
		DWORD dwEffect = 0;
		hr = DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);
		if (SUCCEEDED(hr)) {
			MNEMOSY_TRACE("DropHandler::BeginDrag::DoDragDrop() Succeeded");
		}
		else {
			Check_HRESULT_Error("DropHandler::BeginDrag::DoDragDrop():", hr);
		}

		// release dataObject and dropSource
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

	STDMETHODIMP_(HRESULT __stdcall) DropSource::QueryInterface(REFIID riid, void** ppvObj) {

		if (!ppvObj) {
			//MNEMOSY_TRACE("DropSource::QuearyInterface: E_INVALIDARG");
			return E_INVALIDARG;
		}

		*ppvObj = NULL;
		if (riid == IID_IUnknown  || riid == IID_IDropSource || riid == CLSID_DropSource) {
			// Increment the reference count and return the pointer.
			AddRef();
			*ppvObj = reinterpret_cast<void*>(this);
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
		//MNEMOSY_TRACE("ContinueDrag CANCEL");
			return DRAGDROP_S_CANCEL;
		}
		else if (!haveMouseButton) {
		//MNEMOSY_TRACE("ContinueDrag DROP");
			return DRAGDROP_S_DROP;
		}

		//MNEMOSY_TRACE("ContinueDrag S_OK");
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

	FileDataObject::FileDataObject() {
		//MNEMOSY_TRACE("IDataObject::Constructor:")
	}

	FileDataObject::~FileDataObject() {

		if (!m_filePaths.empty())
			m_filePaths.clear();
		//MNEMOSY_TRACE("IDataObject::Destructor:")
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::QueryInterface(REFIID riid, void** ppvObj) {

		if (!ppvObj) {
			//MNEMOSY_TRACE("IDataObject::QuearyInterface: E_INVALIDARG");
			return E_INVALIDARG;
		}

		*ppvObj = NULL;
		if (riid == IID_IUnknown || riid == IID_IDataObject || riid == CLSID_FileDataObject) {
			// Increment the reference count and return the pointer.
			AddRef();
			*ppvObj = reinterpret_cast<void*>(this);// (LPVOID)this;
			//MNEMOSY_TRACE("IDataObject::QuearyInterface: NOERROR");
			return S_OK;
		}
		
		//MNEMOSY_TRACE("IDataObject::QuearyInterface: E_NOINTERFACE");
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG __stdcall) FileDataObject::AddRef() {

		InterlockedIncrement(&m_cRef);
		//MNEMOSY_TRACE("IDataObject::AddRef: Count: {}", (int)m_cRef);
		return m_cRef;
	}

	STDMETHODIMP_(ULONG __stdcall) FileDataObject::Release() {

		ULONG ulRefCount = InterlockedDecrement(&m_cRef);
		//MNEMOSY_TRACE("IDataObject::Release: Count: {}",int(m_cRef));
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
					// clear the vector after we have copied it to the end of DROPFILES.
					unicodeFilePaths.clear();

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

		//MNEMOSY_DEBUG("IDataObject::GetDataHere: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::QueryGetData(FORMATETC* pFormatetc) {


		if (pFormatetc->cfFormat == CF_HDROP && pFormatetc->tymed == TYMED_HGLOBAL) {

			//MNEMOSY_TRACE("IDataObject::QueryGetData: S_OK");
			return S_OK;
		}

		//MNEMOSY_TRACE("IDataObject::QueryGetData: DV_E_FORMATETC");
		return DV_E_FORMATETC;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::GetCanonicalFormatEtc(FORMATETC* pFormatetcIn, FORMATETC* pFormatetcOut) {
		//MNEMOSY_TRACE("IDataObject::GetCanonicalFormatEtc: NotImpl");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::SetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium, BOOL fRelease) {
		
		//MNEMOSY_DEBUG("FileDataObject::SetData:");
		return E_NOTIMPL;		

		/*
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
		*/

		//MNEMOSY_TRACE("IDataObject::SetData: NotImpl");
	}

	STDMETHODIMP_(HRESULT __stdcall) FileDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc) {

		if (dwDirection == DATADIR_GET) {
			
			//MNEMOSY_DEBUG("IDataObject::EnumFormatEtc: CreateInstance of EnumFormatEtc");
			DataEnumFormatEtc* formatic;
			HRESULT hr = CoCreateInstance(CLSID_DataFormatEtc, NULL, CLSCTX_INPROC_SERVER, IID_IEnumFORMATETC, reinterpret_cast<void**>(&formatic));
			Check_HRESULT_Error("IDataObject::EnumFormatEtc::CoCreateInstance for DataFormatEtc", hr);
			
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
	
	DataEnumFormatEtc::DataEnumFormatEtc( ) {

		m_nIndex = 0;
		m_nNumFormats = 1; // hardcoded for now


		m_pFormatetc = new FORMATETC();
		m_pFormatetc->cfFormat = CF_HDROP;
		m_pFormatetc->dwAspect = DVASPECT_CONTENT;
		m_pFormatetc->ptd = NULL;
		m_pFormatetc->tymed = TYMED_HGLOBAL;
		m_pFormatetc->lindex = -1;
	}

	DataEnumFormatEtc::~DataEnumFormatEtc() {

		//MNEMOSY_TRACE("DataEnumFormatEtc::Release: Destructor");
		if (m_pFormatetc) {
			delete m_pFormatetc;
			m_pFormatetc = nullptr;
		}
	}

	HRESULT __stdcall DataEnumFormatEtc::QueryInterface(REFIID riid, void** ppvObj) {
		
		if (!ppvObj) {
			//MNEMOSY_TRACE("DataEnumFormatEtc::QuearyInterface: E_INVALIDARG");
			return E_INVALIDARG;
		}

		if (riid == IID_IUnknown || riid == IID_IEnumFORMATETC || riid == CLSID_DataFormatEtc) {
			// Increment the reference count and return the pointer.
			*ppvObj =  reinterpret_cast<void*>(this); //(LPVOID)this; 
			AddRef();
			//MNEMOSY_TRACE("DataEnumFormatEtc::QueryInterface: S_OK");
			return S_OK;

		}

		*ppvObj = NULL;
		//MNEMOSY_TRACE("DataEnumFormatEtc::QueryInterface: E_NOINTERFACE");
		return E_NOINTERFACE;
	}

	ULONG __stdcall DataEnumFormatEtc::AddRef() {

		InterlockedIncrement(&m_cRef);
		//MNEMOSY_TRACE("DataEnumFormatEtc::AddRef: RefCount: {}",int(m_cRef));
		return m_cRef;
	}

	ULONG __stdcall DataEnumFormatEtc::Release() {

		ULONG ulRefCount = InterlockedDecrement(&m_cRef);
		//MNEMOSY_TRACE("DataEnumFormatEtc::Release: RefCount: {}",int(m_cRef));

		if (0 == m_cRef) {
			delete this;
		}
		return ulRefCount;
	}

	HRESULT __stdcall DataEnumFormatEtc::Clone(IEnumFORMATETC** ppenum) {

		// never seen this method be called
		//MNEMOSY_TRACE("DataEnumFormatEtc::Clone: E_NOTIMPL");
		return E_NOTIMPL;

		/*
		DataEnumFormatEtc* formatic = new DataEnumFormatEtc();
		//MNEMOSY_DEBUG("DataEnumFormatEtc::Clone: CreateInstance of EnumFormatEtc");
		//HRESULT hr = CoCreateInstance(CLSID_DataFormatEtc, NULL, CLSCTX_INPROC_SERVER, IID_IEnumFORMATETC, reinterpret_cast<void**>(&formatic));
		//Check_HRESULT_Error("DataEnumFormatEtc::CoCreateInstance: ",hr);


		formatic->m_nIndex = m_nIndex;
		*ppenum = formatic;
		//formatic->Release();
		
		//MNEMOSY_TRACE("DataEnumFormatEtc::Clone: S_OK");
		return S_OK;
		*/
	}

	HRESULT __stdcall DataEnumFormatEtc::Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched) {

		int count = 0;
		FORMATETC f;
		FORMATETC* p;

		p = rgelt;

		f.ptd = NULL;
		f.dwAspect = DVASPECT_CONTENT;
		f.lindex = -1;


		while (m_nIndex < 2 && celt >0) {

			count++;
			if (m_nIndex == 0) {

				f.cfFormat = CF_HDROP;
				f.tymed = TYMED_HGLOBAL;
			}
			else if (m_nIndex == 1) {
				// i know. okey i know..
				f.cfFormat = CF_HDROP;
				f.tymed = TYMED_HGLOBAL;
			}

			memmove(p, &f, sizeof(FORMATETC));
			celt--;
			m_nIndex++;
			p = p + sizeof(FORMATETC);

		}

		if (pceltFetched == NULL) {
			//MNEMOSY_TRACE("DataEnumFormatEtc::Next: Copied: {} ", count);
			pceltFetched = new ULONG(count);
		}

		if (count > 0) {
			//MNEMOSY_TRACE("DataEnumFormatEtc::Next: S_OK");
			return S_OK;
		}

		//MNEMOSY_TRACE("DataEnumFormatEtc::Next: S_FALSE");
		return S_FALSE;

		//// ============================
		// Code below works on pc (windows 11 - AMD CPU / NVIDIA GPU)
		// But crashes on laptop  (windows 11 - INTEL CPU/GPU)
		/*
		if (celt == 0 || rgelt == 0) {
			MNEMOSY_TRACE("DataEnumFormatEtc::Next: E_INVALIDARG");
			return E_INVALIDARG;
		}

		int copied = 0;
		while (m_nIndex < m_nNumFormats && copied < celt) {

			rgelt = m_pFormatetc;

			if (rgelt->ptd) {
				m_pFormatetc->ptd = reinterpret_cast<DVTARGETDEVICE*>(CoTaskMemAlloc(sizeof(DVTARGETDEVICE)));
				m_pFormatetc->ptd = rgelt->ptd;
			}

			copied++;
			m_nIndex++;
		}


		if (pceltFetched == NULL) {
			MNEMOSY_TRACE("DataEnumFormatEtc::Next: Copied: {} ", copied);
			ULONG* c = new ULONG(copied);
			pceltFetched = c;
		}

		if (copied == celt) {
			MNEMOSY_TRACE("DataEnumFormatEtc::Next: S_OK");
			return S_OK;
		}

		MNEMOSY_TRACE("DataEnumFormatEtc::Next: S_FALSE");
		return S_FALSE;
		*/
	}

	HRESULT __stdcall DataEnumFormatEtc::Reset() {

		//MNEMOSY_TRACE("DataEnumFormatEtc::Reset:");
		m_nIndex = 0;
		return S_OK;
	}

	HRESULT __stdcall DataEnumFormatEtc::Skip(ULONG celt) {
		// Never seen this function be called

		m_nIndex += celt;
		if (m_nIndex <= m_nNumFormats) {
			//MNEMOSY_TRACE("DataEnumFormatEtc::Skip: S_OK");
			return S_OK;
		}
			//MNEMOSY_TRACE("DataEnumFormatEtc::Skip: S_FALSE");
		return S_FALSE;
	}



	// ========= Com Class Factories =========================================
	// =======================================================================
	// It seems i dont need these at all... but keeping them around if it turns out i need them it in the future
	
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

			//MNEMOSY_TRACE("DataObjectClassFactory::CreateInstance");
			FileDataObject* obj = new FileDataObject();
			HRESULT hr = obj->QueryInterface(riid, ppvObj);
			obj->Release();
			return hr;
		}

		//MNEMOSY_TRACE("DataObjectClassFactory::CreateInstance. E_NOINTERFACE");
		return E_NOINTERFACE;
	}
	
	// == EnumFormatEtc Factory
	HRESULT __stdcall EnumFormatEtcClassFactory::QueryInterface(REFIID riid, void** ppvObj) {

		if (riid == IID_IUnknown || riid == IID_IClassFactory ) {
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

		// Assume an error by clearing callers handle
		*ppvObj = 0;
		if (punkOuter) {
			return CLASS_E_NOAGGREGATION;
		}
		else {

			//MNEMOSY_TRACE("EnumFormatEtcClassFactory::CreateInstance:");
			DataEnumFormatEtc* obj = new DataEnumFormatEtc();

			HRESULT hr = obj->QueryInterface(riid, ppvObj);
			obj->Release();
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
			MNEMOSY_WARN("windows com error: {} Failed: {}", prefixMessage, std::string(errorMsg));
		}
	}

	


}// !mnemosy::core
	//==== private methods

#endif // !MNEMOSY_PLATFORM_WINDOW