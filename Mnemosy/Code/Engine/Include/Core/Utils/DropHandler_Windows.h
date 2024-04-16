#ifndef DROP_HANDLER_WINDOWS_H
#define DROP_HANDLER_WINDOWS_H


#include <windows.h>
#include <objbase.h>
#include <initguid.h>

#include <vector>
#include <string>

namespace mnemosy::core {
	class DropManager;
}

namespace mnemosy::core {

	class DropHandler
	{
	public:
		DropHandler();
		~DropHandler();

		void Initialize();
		void Uninitialize();

        void BeginDrag();


        /// <summary>
        /// Set a bool in the drop manager witch is just for cosmetic purposes. it changes the mouse cursor to display when an area is valid to drop files to. Set true to make the area valid.
        /// </summary>
        /// <param name="state"></param>
        void SetDropTargetActive(bool state);

	private:

        DropManager* m_pDropManager = nullptr;

        // if we need them again we need to forward declare them after DropManager at the top of this file
        //DropSourceClassFactory*     m_pDropSourceFactory = nullptr;
        //DataObjectClassFactory*     m_pDataObjectFactory = nullptr;
        //EnumFormatEtcClassFactory*  m_pEnumFormatEtcFactory = nullptr;
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
       
    public:
        FileDataObject();
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
        ULONG m_cRef = 1;
        int m_nNumFormats = 1;

        std::vector<std::string> m_filePaths;
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

       // int m_fIndex = 0;
    private:
        ULONG m_cRef = 1;

        ULONG m_nIndex;
        ULONG m_nNumFormats;
        FORMATETC* m_pFormatetc = nullptr;

    };


    // ========= Com Class Factories =========================================
    // =======================================================================
    // It seems i dont need these at all... but keeping them around if it turns out i need them it in the future
    /*
    class DropSourceClassFactory : public IClassFactory {
    public:

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;

        ULONG STDMETHODCALLTYPE AddRef() override { return 1; }

        ULONG STDMETHODCALLTYPE Release() override { return 1; }

        HRESULT STDMETHODCALLTYPE LockServer(BOOL flock) override { return NOERROR; }

        HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj) override;


    };
    
    class DataObjectClassFactory : public IClassFactory {
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
    */


	// Utility Methods
	static void Check_HRESULT_Error(const char* prefixMessage, HRESULT hr);


}// !mnemosy::core

#endif // !DROP_HANDLER_WINDOWS_H
