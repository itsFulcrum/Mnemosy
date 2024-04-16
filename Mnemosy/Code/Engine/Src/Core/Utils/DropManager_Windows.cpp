#include "Include/Core/Utils/DropManager_Windows.h"

#include "Include/Core/Log.h"


namespace mnemosy::core
{


	STDMETHODIMP_(HRESULT __stdcall) DropManager::QueryInterface(REFIID riid, void** ppvObj) {

		if (!ppvObj) {
			//MNEMOSY_TRACE("DropManager::QuearyInterface: E_INVALIDARG");
			return E_INVALIDARG;
		}

		*ppvObj = NULL;
		if (riid == IID_IUnknown || riid == IID_IDropTarget)
		{
			// Increment the reference count and return the pointer.
			*ppvObj = (LPVOID)this;
			AddRef();
			//MNEMOSY_TRACE("DropManager::QuearyInterface: NOERROR");
			return NOERROR;

		}

		//MNEMOSY_TRACE("DropManager::QuearyInterface: E_NOINTERFACE");
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG __stdcall) DropManager::AddRef()
	{
		InterlockedIncrement(&m_cRef);
		//MNEMOSY_WARN("DropManager::AddRef: ");
		return m_cRef;
	}

	STDMETHODIMP_(ULONG __stdcall) DropManager::Release()
	{
		ULONG ulRefCount = InterlockedDecrement(&m_cRef);
		//MNEMOSY_WARN("DropManager::Release: RefCount: {}", (int)m_cRef);
		if (0 == m_cRef) {
			delete this;
		}
		return ulRefCount;
	}

	HRESULT __stdcall DropManager::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {


		return DROPEFFECT_NONE;

		if (m_CanDrop) {
			//MNEMOSY_WARN("OnDragEnter canDrop");
			return DROPEFFECT_COPY || DROPEFFECT_MOVE;
		}

		//MNEMOSY_WARN("OnDragEnter NoDrop");


		//return E_NOTIMPL;
	}
	HRESULT __stdcall DropManager::DragOver(DWORD grfkeyState, POINTL pt, DWORD* pdwEffect) {

		// this is only implemented like this to make windows set the mouse coursor to show if the area (button) will accept a drop

		if (m_CanDrop) {
			//MNEMOSY_WARN("OnDragOver canDrop");

			return DROPEFFECT_NONE;
			//return DROPEFFECT_COPY || DROPEFFECT_MOVE;
		}

		//MNEMOSY_WARN("OnDragOver");
		return E_NOTIMPL;

	}
	HRESULT __stdcall DropManager::DragLeave(void) {

		//MNEMOSY_WARN("OnDragLeave");

		return E_NOTIMPL;
	}

	HRESULT __stdcall DropManager::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {

		//MNEMOSY_WARN("OnDrop");
		return E_NOTIMPL;
	}

	

}