#ifndef GUI_PANEL_MANAGER_H
#define GUI_PANEL_MANAGER_H

namespace mnemosy::gui
{
	class MainMenuBarGuiPanel;
	class ViewportGuiPanel;
	class SettingsGuiPanel;
	class MaterialEditorGuiPanel;
	class MaterialLibraryGuiPanel;
	class DocumentationGuiPanel;
}

namespace mnemosy::gui
{

	class GuiPanelManager {

	public:
		GuiPanelManager();
		~GuiPanelManager();

		ViewportGuiPanel&			GetViewportPanel()			{ return *m_pViewportPanel; }
		SettingsGuiPanel&			GetSettingsPanel()			{ return *m_pSettingsPanel; }
		MaterialEditorGuiPanel&		GetMaterialEditorPanel()	{ return *m_pMaterialEditorPanel; }
		MaterialLibraryGuiPanel&	GetMaterialLibraryPanel()	{ return *m_pMaterialLibraryPanel; }
		DocumentationGuiPanel&		GetDocumentationPanel()		{ return *m_pDocumentationPanel; }

	private:
		MainMenuBarGuiPanel*		m_pMainMenuBarPanel			= nullptr;
		ViewportGuiPanel*			m_pViewportPanel			= nullptr;
		SettingsGuiPanel*			m_pSettingsPanel			= nullptr;
		MaterialEditorGuiPanel*		m_pMaterialEditorPanel		= nullptr;
		MaterialLibraryGuiPanel*	m_pMaterialLibraryPanel		= nullptr;
		DocumentationGuiPanel*		m_pDocumentationPanel		= nullptr;

	};

}

#endif // !GUI_PANEL_MANAGER_H
