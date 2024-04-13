#include "Include/Core/Utils/DropManager_Windows.h"

#include "Include/Core/Log.h"

namespace mnemosy::core
{
	HRESULT __stdcall DropManager::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {

		MNEMOSY_WARN("OnDragEnter");

		return E_NOTIMPL;
	}
	HRESULT __stdcall DropManager::DragOver(DWORD grfkeyState, POINTL pt, DWORD* pdwEffect) {

		MNEMOSY_WARN("OnDragOver");

		return E_NOTIMPL;
	}
	HRESULT __stdcall DropManager::DragLeave(void) {

		MNEMOSY_WARN("OnDragLeave");

		return E_NOTIMPL;
	}

	HRESULT __stdcall DropManager::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {

		MNEMOSY_WARN("OnDrop");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(HRESULT __stdcall) DropManager::QueryInterface(REFIID riid, void** ppvObj)
	{
		MNEMOSY_WARN("DropManager::QueryInterface: ");
		return E_NOTIMPL;
	}

	STDMETHODIMP_(ULONG __stdcall) DropManager::AddRef()
	{
		MNEMOSY_WARN("DropManager::AddRef: ");
		InterlockedIncrement(&m_cRef);
		return m_cRef;
	}

	STDMETHODIMP_(ULONG __stdcall) DropManager::Release()
	{
		MNEMOSY_WARN("DropManager::Release: ");
		ULONG ulRefCount = InterlockedDecrement(&m_cRef);

		if (0 == m_cRef)
		{
			delete this;
		}
		return ulRefCount;
	}

}