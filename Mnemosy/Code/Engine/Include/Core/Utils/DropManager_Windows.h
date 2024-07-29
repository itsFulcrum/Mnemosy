#ifndef DROP_MANAGER_H
#define DROP_MANAGER_H


// TODO check witch of these are actually neccesary

#include <windows.h>
#include <objbase.h>
#include <initguid.h>

namespace mnemosy::core
{

	//71a260bb-52b0-4016-8125-07ba49bf9088
	DEFINE_GUID(CLSID_DropManager, 0x71a260bb, 0x52b0, 0x4016, 0x81, 0x25, 0x07, 0xba, 0x49, 0xbf, 0x90, 0x88);


	class DropManager : public IDropTarget
	{
		ULONG m_cRef = 0;

		bool m_CanDrop = true;


	public:



		STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);

		STDMETHODIMP_(ULONG) AddRef();

		STDMETHODIMP_(ULONG) Release();
		

		virtual HRESULT STDMETHODCALLTYPE DragEnter(__RPC__in_opt IDataObject* pDataObj, DWORD grfKeyState, POINTL pt,__RPC__inout DWORD* pdwEffect) override;
		
		virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfkeyState,POINTL pt, __RPC__inout DWORD* pdwEffect) override;
		
		virtual HRESULT STDMETHODCALLTYPE DragLeave(void) override;
		
		virtual HRESULT STDMETHODCALLTYPE Drop(__RPC__in_opt IDataObject * pDataObj,DWORD grfKeyState,POINTL pt,__RPC__inout DWORD* pdwEffect) override;
		

		void SetDropActive(bool state) { m_CanDrop = state; }
		
	};



} // !mnemosy::core


#endif // !DROP_MANAGER_H
