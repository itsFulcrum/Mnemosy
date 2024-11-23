#ifndef DROP_HANDLER_WINDOWS_H
#define DROP_HANDLER_WINDOWS_H

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

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

    // GUIDs for registering interfaces with windows com api
    // {EF7660D0-0FCE-4A79-BBC1-DDDA71BDA23D}
    DEFINE_GUID(CLSID_DropSource,
        0xef7660d0, 0xfce, 0x4a79, 0xbb, 0xc1, 0xdd, 0xda, 0x71, 0xbd, 0xa2, 0x3d);
    // {163438E1-8AB4-4CB1-8EE5-D4EBC5FF9626}
    DEFINE_GUID(CLSID_FileDataObject,
        0x163438e1, 0x8ab4, 0x4cb1, 0x8e, 0xe5, 0xd4, 0xeb, 0xc5, 0xff, 0x96, 0x26);
    // {5519EBB5-2116-43F6-A2ED-59F3C32188B3}
    DEFINE_GUID(CLSID_DataFormatEtc,
        0x5519ebb5, 0x2116, 0x43f6, 0xa2, 0xed, 0x59, 0xf3, 0xc3, 0x21, 0x88, 0xb3);
    
	class DropHandler
	{
	public:
		DropHandler() = default;
		~DropHandler() = default;

		void Init(GLFWwindow& window);
		void Shutdown();

        void BeginDrag(std::vector<std::string>& filesToDrag);


        /// <summary>
        /// Set a bool in the drop manager witch is just for cosmetic purposes. it changes the mouse cursor to display when an area is valid to drop files to. Set true to make the area valid.
        /// </summary>
        /// <param name="state"></param>
        void SetDropTargetActive(bool state);

	private:
        DropManager* m_pDropManager = nullptr;

        // It seems i dont need the factories at all... but it's probably a good idea to register with windows
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

    private:
        ULONG m_cRef = 1;

        ULONG m_nIndex;
        ULONG m_nNumFormats;
        FORMATETC* m_pFormatetc = nullptr;

    };


    // ========= Com Class Factories =========================================
    // =======================================================================
    // It seems i dont need the factories at all... but it's probably a good idea to register with windows
    
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
    

    // ========= Utility Methods =============================================
    // =======================================================================

	static void Check_HRESULT_Error(const char* prefixMessage, HRESULT hr);


}// !mnemosy::core

#endif // !DROP_HANDLER_WINDOWS_H
