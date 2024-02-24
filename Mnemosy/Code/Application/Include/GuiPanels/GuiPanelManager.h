#ifndef GUI_PANEL_MANAGER_H
#define GUI_PANEL_MANAGER_H

namespace mnemosy::gui
{
	class MainMenuBarGuiPanel;
	class ViewportGuiPanel;
	class GlobalSettingsGuiPanel;
	class SceneSettingsGuiPanel;
	class MaterialEditorGuiPanel;
}

namespace mnemosy::gui
{
	class GuiPanelManager
	{
	public:
		GuiPanelManager();
		~GuiPanelManager();




		ViewportGuiPanel& GetViewportPanel() { return *m_pViewportPanel; }
		GlobalSettingsGuiPanel& GetGlobalSettingsPanel() { return *m_pGlobalSettingsPanel; }
		SceneSettingsGuiPanel& GetSceneSettingsPanel() { return *m_pSceneSettingsPanel; }
		MaterialEditorGuiPanel& GetMaterialEditorPanel() { return *m_pMaterialEditorPanel; }


	private:
		MainMenuBarGuiPanel* m_pMainMenuBarPanel = nullptr;
		ViewportGuiPanel* m_pViewportPanel = nullptr;
		GlobalSettingsGuiPanel* m_pGlobalSettingsPanel = nullptr;
		SceneSettingsGuiPanel* m_pSceneSettingsPanel = nullptr;
		MaterialEditorGuiPanel* m_pMaterialEditorPanel = nullptr;


	};

}

#endif // !GUI_PANEL_MANAGER_H
