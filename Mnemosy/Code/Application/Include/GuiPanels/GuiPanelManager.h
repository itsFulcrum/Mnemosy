#ifndef GUI_PANEL_MANAGER_H
#define GUI_PANEL_MANAGER_H

namespace mnemosy::gui
{
	class ViewportGuiPanel;
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



	private:


		ViewportGuiPanel* m_pViewportPanel = nullptr;
		SceneSettingsGuiPanel* m_pSceneSettingsPanel = nullptr;
		MaterialEditorGuiPanel* m_pMaterialEditorPanel = nullptr;


	};

}

#endif // !GUI_PANEL_MANAGER_H
