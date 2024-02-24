#ifndef APPLICATION_H
#define APPLICATION_H

#include "Engine/Include/MnemosyEngine.h"

namespace mnemosy::input
{
	class CameraInputController;
	class SceneInputController;
}
namespace mnemosy::gui
{
	class GuiPanelManager;
}
namespace mnemosy
{
#define ENGINE_INSTANCE() Application::GetInstance().GetEngine().GetInstance()

	class Application
	{
	private:
		Application();
		static Application* m_sInstance;


	public:
		static Application& GetInstance();


		~Application() = default;

		void Initialize();

		void Run();

		void Shutdown();

		MnemosyEngine& GetEngine();
		gui::GuiPanelManager& GetGuiPanelManager() { return *m_pGuiPanelManager; }

	private:

		MnemosyEngine& m_mnemosyEngine = MnemosyEngine::GetInstance();



		input::CameraInputController* m_cameraController = nullptr;
		input::SceneInputController* m_sceneInputController = nullptr;

		gui::GuiPanelManager* m_pGuiPanelManager = nullptr;
	};


}

#endif // !APPLICATION_H
