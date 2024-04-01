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

#include <vector>
#include <string>



namespace mnemosy::core
{

	class FileDialogs
	{
	public:
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
		static std::string SelectFolder(const char* filter);


		static void StartDrag();

	};

    /*
    class FileDataObject : public IDataObject {
    private:
        std::vector<std::string> m_filePaths;

    public:
        FileDataObject(const std::vector<std::string>& filePaths) : m_filePaths(filePaths) {}

        //// Implement the IDataObject interface methods
        STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) {
            // Implement according to your needs
            return E_NOTIMPL;
        }

        STDMETHODIMP_(ULONG) AddRef() {
            // Implement according to your needs
            return E_NOTIMPL;
        }

        STDMETHODIMP_(ULONG) Release() {
            // Implement according to your needs
            return E_NOTIMPL;
        }

        STDMETHODIMP GetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium) {
            if (pFormatetc->cfFormat == CF_HDROP && pFormatetc->tymed == TYMED_HGLOBAL) {
                // Prepare file drop data
                //std::vector<wchar_t> buffer;
                //for (const auto& filePath : m_filePaths) {
                //    buffer.insert(buffer.end(), filePath.begin(), filePath.end());
                //    buffer.push_back(L'\0');
                //}
                //buffer.push_back(L'\0'); // Double null-terminated
                //
                //// Allocate memory for file drop structure
                //HGLOBAL hGlobal = GlobalAlloc(GHND, sizeof(DROPFILES) + (buffer.size() * sizeof(wchar_t)));
                //
                //DROPFILES* pDropFiles = reinterpret_cast<DROPFILES*>(GlobalLock(hGlobal));
                //pDropFiles->pFiles = sizeof(DROPFILES);
                //pDropFiles->fWide = TRUE; // Indicate Unicode file paths
                //memcpy((char*)pDropFiles + sizeof(DROPFILES), buffer.data(), buffer.size() * sizeof(wchar_t));
                //GlobalUnlock(hGlobal);
                //
                
                UINT uBuffSize = 0;

                for (int i = 0; i < m_filePaths.size(); i++) {
                    uBuffSize += lstrlen(m_filePaths[i].c_str()) + 1;

                }
                uBuffSize = sizeof(DROPFILES) + sizeof(TCHAR) * (uBuffSize + 1);


                // Allocate memory from the heap for the DROPFILES struct.
                HGLOBAL hGlobal = GlobalAlloc(GHND, uBuffSize);;

                //if (NULL == hGlobal)
                    //return;


                DROPFILES* pDrop = reinterpret_cast<DROPFILES*>(GlobalLock(hGlobal));
                //pDrop = (DROPFILES*)GlobalLock(hGlobal);

                //if (NULL == pDrop) {
                  //  GlobalFree(hGlobal);
                    //return;
                //}


                // Fill in the DROPFILES struct.
                pDrop->pFiles = sizeof(DROPFILES);
                pDrop->fWide = TRUE;

                //#ifdef _UNICODE
                //		// If we're compiling for Unicode, set the Unicode flag in the struct to
                //		// indicate it contains Unicode strings.
                //#endif


                        //Now we can copy all of the filenames into memory, and then unlock the buffer.

                TCHAR* pszBuff;

                // Copy all the filenames into memory after
                // the end of the DROPFILES struct.
                //pos = lsDraggedFiles.GetHeadPosition();
                pszBuff = (TCHAR*)(LPBYTE(pDrop) + sizeof(DROPFILES));

                //while (NULL != pos)
                //{
                //	lstrcpy(pszBuff, (LPCTSTR)lsDraggedFiles.GetNext(pos));
                //	pszBuff = 1 + _tcschr(pszBuff, '\0');
                //}
                for (int i = 0; i < m_filePaths.size(); i++) {
                    //uBuffSize += lstrlen(filesToDrag[i].c_str()) + 1;
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

        STDMETHODIMP GetDataHere(FORMATETC* pFormatetc, STGMEDIUM* pmedium) {
            return E_NOTIMPL;
        }

        STDMETHODIMP QueryGetData(FORMATETC* pFormatetc) {
            if (pFormatetc->cfFormat == CF_HDROP && pFormatetc->tymed == TYMED_HGLOBAL) {
                return S_OK;
            }
            return DV_E_FORMATETC;
        }

        STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pFormatetcIn, FORMATETC* pFormatetcOut) {
            return E_NOTIMPL;
        }

        STDMETHODIMP SetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium, BOOL fRelease) {
            return E_NOTIMPL;
        }

        STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc) {
            return E_NOTIMPL;
        }

        STDMETHODIMP DAdvise(FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) {
            return E_NOTIMPL;
        }

        STDMETHODIMP DUnadvise(DWORD dwConnection) {
            return E_NOTIMPL;
        }

        STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppenumAdvise) {
            return E_NOTIMPL;
        }
    };
    */

}

#endif // !PLATFORM_UTILS_WINDOWS_H
