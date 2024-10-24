#ifndef APPLICATION_H
#define APPLICATION_H

namespace mnemosy
{
	class MnemosyEngine;
}
namespace mnemosy::input
{
	class CameraInputController;
	class SceneInputController;
}
namespace mnemosy::gui
{
	class GuiPanelManager;
}

namespace mnemosy::systems
{
	class GuiUserSettingsManager;
}

namespace mnemosy
{
	// SINGLETON CLASS
	class Application {


	private:
		Application();
		static Application* m_sInstance;

	public:
		static Application& GetInstance();
		~Application();

		void Initialize();
		void Run();
		void Shutdown();

		gui::GuiPanelManager& GetGuiPanelManager() { return *m_pGuiPanelManager; }
		systems::GuiUserSettingsManager& GetGuiUserSettingsManager() {return *m_pGuiUserSettingsManager;}
	private:

		MnemosyEngine& m_mnemosyEngine;

		input::CameraInputController* m_pCameraController = nullptr;
		input::SceneInputController* m_pSceneInputController = nullptr;
		gui::GuiPanelManager* m_pGuiPanelManager = nullptr;
		systems::GuiUserSettingsManager* m_pGuiUserSettingsManager = nullptr;



	};
}
#endif // !APPLICATION_H
