#ifndef GUI_PANEL_MANAGER_H
#define GUI_PANEL_MANAGER_H

namespace mnemosy::gui
{
	class MainMenuBarGuiPanel;
	class ViewportGuiPanel;
	class SceneSettingsGuiPanel;
	class MaterialEditorGuiPanel;
	class MaterialLibraryGuiPanel;
	class DocumentationGuiPanel;
}

namespace mnemosy::gui
{
	class GuiPanelManager
	{
	public:
		GuiPanelManager();
		~GuiPanelManager();

		ViewportGuiPanel& GetViewportPanel() { return *m_pViewportPanel; }
		SceneSettingsGuiPanel& GetSceneSettingsPanel() { return *m_pSceneSettingsPanel; }
		MaterialEditorGuiPanel& GetMaterialEditorPanel() { return *m_pMaterialEditorPanel; }
		MaterialLibraryGuiPanel& GetMaterialLibraryPanel() { return *m_pMaterialLibraryPanel; }
		DocumentationGuiPanel& GetDocumentationPanel() { return *m_pDocumentationPanel; }

	private:
		MainMenuBarGuiPanel* m_pMainMenuBarPanel = nullptr;
		ViewportGuiPanel* m_pViewportPanel = nullptr;
		SceneSettingsGuiPanel* m_pSceneSettingsPanel = nullptr;
		MaterialEditorGuiPanel* m_pMaterialEditorPanel = nullptr;
		MaterialLibraryGuiPanel* m_pMaterialLibraryPanel = nullptr;
		DocumentationGuiPanel* m_pDocumentationPanel = nullptr;



	};

}

#endif // !GUI_PANEL_MANAGER_H
