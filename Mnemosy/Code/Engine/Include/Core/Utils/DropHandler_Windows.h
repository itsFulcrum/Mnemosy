#ifndef DROP_HANDLER_WINDOWS_H
#define DROP_HANDLER_WINDOWS_H


#include <windows.h>
#include <objbase.h>
#include <initguid.h>

#include <vector>
#include <string>

namespace mnemosy::core {
	class DropManager;
    class DropSourceClassFactory;
    class DataObjectClassFactory;
    class EnumFormatEtcClassFactory;
}

namespace mnemosy::core {

    //7f0276eb - 4541 - 41f4 - a1cb - 573911e9af42
    DEFINE_GUID(CLSID_FileDataObject, 0x7f0276eb, 0x4541, 0x41f4, 0xa1, 0xcb, 0x57, 0x39, 0x11, 0xe9, 0xaf, 0x42);

    // 39749376-11c1-4b4e-93c5-0d4ec38f9136
    DEFINE_GUID(CLSID_DropSource, 0x39749376, 0x11c1, 0x4b4e, 0x93, 0xc5, 0x0d, 0x4e, 0xc3, 0x8f, 0x91, 0x36);

    // cda0beb2-1aed-4aab-8b94-ac3c4a9be5ca
    DEFINE_GUID(CLSID_DataFormatEtc, 0xcda0beb2, 0x1aed, 0x4aab, 0x8b, 0x94, 0xac, 0x3c, 0x4a, 0x9b, 0xe5, 0xca);

    //static UINT CF_filename;


	class DropHandler
	{
	public:
		DropHandler();
		~DropHandler();

		void RegisterDropTarget();
		void Initialize();
		void Uninitialize();

        void BeginDrag();


        void SetDropTargetActive(bool state);
	private:

        DropManager* m_pDropManager = nullptr;

        DropSourceClassFactory*     m_pDropSourceFactory = nullptr;
        DataObjectClassFactory*     m_pDataObjectFactory = nullptr;
        EnumFormatEtcClassFactory*  m_pEnumFormatEtcFactory = nullptr;
	};

    // ========= Interface Implementations ====================================
    // ========================================================================
    
    // ==== DropSource 
    class DropSource : public IDropSource
    {
    public:
        DropSource();
        ~DropSource();

        virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override;

        virtual STDMETHODIMP_(ULONG) AddRef() override;

        virtual STDMETHODIMP_(ULONG) Release() override;

        virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override;

        virtual HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect) override;

    private:
        ULONG m_cRef = 1;
    };

    // ==== FileDataObject

    class FileDataObject : public IDataObject {
    private:
        // TODO: make sure to clear this at some point
        std::vector<std::string> m_filePaths; 

        ULONG m_cRef = 1;
    public:
        FileDataObject(FORMATETC* fmtetc,STGMEDIUM* stgmed);
        ~FileDataObject();

        STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override;

        STDMETHODIMP_(ULONG) AddRef() override;

        STDMETHODIMP_(ULONG) Release() override;

        STDMETHODIMP GetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium);

        STDMETHODIMP GetDataHere(FORMATETC* pFormatetc, STGMEDIUM* pmedium);

        STDMETHODIMP QueryGetData(FORMATETC* pFormatetc);

        STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pFormatetcIn, FORMATETC* pFormatetcOut);

        STDMETHODIMP SetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium, BOOL fRelease);

        STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc);

        STDMETHODIMP DAdvise(FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection);

        STDMETHODIMP DUnadvise(DWORD dwConnection);

        STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppenumAdvise);

        void Init(const std::vector<std::string>& filePaths);

    private:
        bool LookupFormatEtc(FORMATETC* pfmtetc);

        int m_nNumFormats = 0;
        STGMEDIUM* m_pstgmed = nullptr;
        FORMATETC* m_pfmtetc = nullptr;
    };


    // ==== DataEnumFormatEtc

    class DataEnumFormatEtc : public IEnumFORMATETC {

    public:

        DataEnumFormatEtc();
        ~DataEnumFormatEtc();

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;

        ULONG STDMETHODCALLTYPE AddRef() override;

        ULONG STDMETHODCALLTYPE Release() override;


        HRESULT STDMETHODCALLTYPE Clone(IEnumFORMATETC** ppenum) override;

        HRESULT STDMETHODCALLTYPE Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched) override;

        HRESULT STDMETHODCALLTYPE Reset() override;

        HRESULT STDMETHODCALLTYPE Skip(ULONG celt) override;
        
        HRESULT DeepCopyFormatEtx(FORMATETC* dest,FORMATETC* source);

        int m_fIndex = 0;
    private:
        ULONG m_cRef = 1;

        ULONG m_nIndex;
        ULONG m_nNumFormats;
        FORMATETC* m_pFormatetc = nullptr;

    };


    // ========= Com Class Factories =========================================
    // =======================================================================

    class DataObjectClassFactory : public IClassFactory {
    public:

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;

        ULONG STDMETHODCALLTYPE AddRef() override { return 1; }

        ULONG STDMETHODCALLTYPE Release() override { return 1; }

        HRESULT STDMETHODCALLTYPE LockServer(BOOL flock) override { return NOERROR; }

        HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj) override;

    };

    class DropSourceClassFactory : public IClassFactory {
    public:

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;

        ULONG STDMETHODCALLTYPE AddRef() override { return 1; }

        ULONG STDMETHODCALLTYPE Release() override { return 1; }

        HRESULT STDMETHODCALLTYPE LockServer(BOOL flock) override { return NOERROR; }

        HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj) override;


    };

    class EnumFormatEtcClassFactory : public IClassFactory {
    public:

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;

        ULONG STDMETHODCALLTYPE AddRef() override { return 1; }

        ULONG STDMETHODCALLTYPE Release() override { return 1; }

        HRESULT STDMETHODCALLTYPE LockServer(BOOL flock) override { return NOERROR; }

        HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj) override;

    };


	// Utility Methods
	static void Check_HRESULT_Error(const char* prefixMessage, HRESULT hr);


}// !mnemosy::core

#endif // !DROP_HANDLER_WINDOWS_H
