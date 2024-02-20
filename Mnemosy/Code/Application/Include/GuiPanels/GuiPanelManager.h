#ifndef GUI_PANEL_MANAGER_H
#define GUI_PANEL_MANAGER_H

namespace mnemosy::gui
{
	class ViewportGuiPanel;
	class SceneSettingsGuiPanel;
}

namespace mnemosy::gui
{
	class GuiPanelManager
	{
	public:
		GuiPanelManager();
		~GuiPanelManager();



	private:


		ViewportGuiPanel* m_viewportPanel = nullptr;
		SceneSettingsGuiPanel* m_sceneSettingsPanel = nullptr;

	};

}

#endif // !GUI_PANEL_MANAGER_H
