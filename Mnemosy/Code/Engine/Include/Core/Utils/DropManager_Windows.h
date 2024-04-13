#ifndef DROP_MANAGER_H
#define DROP_MANAGER_H



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
	class DropManager : public IDropTarget
	{
		ULONG m_cRef = 0;
	public:


		virtual HRESULT STDMETHODCALLTYPE DragEnter(__RPC__in_opt IDataObject* pDataObj, DWORD grfKeyState, POINTL pt,__RPC__inout DWORD* pdwEffect) override;
		
		virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfkeyState,POINTL pt, __RPC__inout DWORD* pdwEffect) override;
		
		virtual HRESULT STDMETHODCALLTYPE DragLeave(void) override;
		
		virtual HRESULT STDMETHODCALLTYPE Drop(__RPC__in_opt IDataObject * pDataObj,DWORD grfKeyState,POINTL pt,__RPC__inout DWORD* pdwEffect) override;
		
		
		STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);

		STDMETHODIMP_(ULONG) AddRef();

		STDMETHODIMP_(ULONG) Release();
	};



} // !mnemosy::core


#endif // !DROP_MANAGER_H
