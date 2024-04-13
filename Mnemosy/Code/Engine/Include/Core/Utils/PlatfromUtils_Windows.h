#ifndef PLATFORM_UTILS_WINDOWS_H
#define PLATFORM_UTILS_WINDOWS_H

//#include <Windows.h>
#include <windows.h>
#include <commdlg.h>
#include <shobjidl_core.h>
#include <shlobj_core.h>
#include <Shellapi.h>
#include <objidl.h>
#include <TCHAR.H>
#include <oleidl.h>

#include <objbase.h>
#include <initguid.h>


#include <vector>
#include <string>



namespace mnemosy::core
{
//7f0276eb - 4541 - 41f4 - a1cb - 573911e9af42
DEFINE_GUID(CLSID_FileDataObject,0x7f0276eb,0x4541,0x41f4,0xa1,0xcb, 0x57, 0x39, 0x11, 0xe9, 0xaf, 0x42);
//71a260bb-52b0-4016-8125-07ba49bf9088
//DEFINE_GUID(IID_FileDataObject,0x71a260bb,0x52b0,0x4016,0x81,0x25, 0x07, 0xba, 0x49, 0xbf, 0x90, 0x88);

// 39749376-11c1-4b4e-93c5-0d4ec38f9136
DEFINE_GUID(CLSID_DropSource, 0x39749376, 0x11c1, 0x4b4e, 0x93, 0xc5, 0x0d, 0x4e, 0xc3, 0x8f, 0x91, 0x36);

// cda0beb2-1aed-4aab-8b94-ac3c4a9be5ca
DEFINE_GUID(CLSID_DataFormatEtc,0xcda0beb2, 0x1aed, 0x4aab, 0x8b, 0x94, 0xac, 0x3c, 0x4a, 0x9b, 0xe5, 0xca);

    class DropManager;

    static void CheckComError(const char* ClassAndfunctionName,HRESULT hr);

    static UINT CF_filename;
	
    
    class FileDialogs
	{
	public:

		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
		static std::string SelectFolder(const char* filter);

        static void RegisterDropManager(DropManager* dropManager);
		static void StartDrag();
	};



    class DropS : public IDropSource
    {
    public:
        

        virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override;

        virtual STDMETHODIMP_(ULONG) AddRef() override;
        
        virtual STDMETHODIMP_(ULONG) Release() override;

        virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override;
          
        virtual HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect) override;
    private:
        ULONG m_cRef = 1;
    };
    
    class FileDataObject : public IDataObject {
    private:
        std::vector<std::string> m_filePaths;

        ULONG m_cRef = 1;
    public:


        FileDataObject(){}


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
    };

    // Factories
    class DataObjectClassFactory : public IClassFactory {
    public:

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override {
            if (riid != IID_IUnknown && riid != IID_IClassFactory) {
                *ppvObj = 0;
                return E_NOINTERFACE;
            }

            *ppvObj = this;
            AddRef();
            return NOERROR;
        }

        ULONG STDMETHODCALLTYPE AddRef() override { return 1; }

        ULONG STDMETHODCALLTYPE Release() override { return 1; }

        HRESULT STDMETHODCALLTYPE LockServer(BOOL flock) override {
            return NOERROR;
        }
        HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj) override;


    };

    class DropSourceClassFactory : public IClassFactory {
    public:

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;

        ULONG STDMETHODCALLTYPE AddRef() override { return 1; }

        ULONG STDMETHODCALLTYPE Release() override { return 1; }

        HRESULT STDMETHODCALLTYPE LockServer(BOOL flock) override {
            return NOERROR;
        }
        HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj) override;


    };

    class EnumFormatEtcClassFactory : public IClassFactory {
    public:

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;

        ULONG STDMETHODCALLTYPE AddRef() override { return 1; }

        ULONG STDMETHODCALLTYPE Release() override { return 1; }

        HRESULT STDMETHODCALLTYPE LockServer(BOOL flock) override {
            return NOERROR;
        }
        HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvObj) override;


    };


    class DataEnumFormatEtc : public IEnumFORMATETC {
    private:
        ULONG m_cRef = 1;
    public:
        int m_fIndex = 0;

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;

        ULONG STDMETHODCALLTYPE AddRef() override;

        ULONG STDMETHODCALLTYPE Release() override;


        HRESULT STDMETHODCALLTYPE Clone(IEnumFORMATETC** ppenum) override;
        
        HRESULT STDMETHODCALLTYPE Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched) override;

        HRESULT STDMETHODCALLTYPE Reset() override;

        HRESULT STDMETHODCALLTYPE Skip(ULONG celt) override;

    };
}

#endif // !PLATFORM_UTILS_WINDOWS_H
