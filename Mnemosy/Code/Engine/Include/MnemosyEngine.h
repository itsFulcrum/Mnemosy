#ifndef MNEMOSY_ENGINE_H
#define MNEMOSY_ENGINE_H

#include <memory>


namespace mnemosy::core
{
	class Clock;
	class Logger;
	class Window;
	class FileDirectories;
	class DropHandler;
}

namespace mnemosy::systems
{
	class InputSystem;
	class SkyboxAssetRegistry;
	class MaterialLibraryRegistry;
	class ThumbnailManager;
	class TextureGenerationManager;
	class ExportManager;
}

namespace mnemosy::graphics
{
	class Camera;
	class Skybox;
	class Renderer;
	class ImageBasedLightingRenderer;
	class Scene;
	class ThumbnailScene;
}
namespace mnemosy::gui
{
	class UserInterface;
}

namespace mnemosy 
{
	
	class MnemosyEngine
	{
	private:
		MnemosyEngine();
		static MnemosyEngine* m_sInstance;

	public:
		~MnemosyEngine();

		static MnemosyEngine& GetInstance();
		
		void Initialize(const char* WindowTitle);
		void Run();
		void Shutdown();


		// Getters
		core::Window& GetWindow()					{ return *m_pWindow; }
		core::FileDirectories& GetFileDirectories() { return *m_pFileDirectories; }
		core::Clock& GetClock()						{ return *m_pClock; }
		core::DropHandler& GetDropHandler()			{ return *m_pDropHandler; }
		systems::InputSystem& GetInputSystem() { return *m_pInputSystem; }
		systems::SkyboxAssetRegistry& GetSkyboxAssetRegistry() { return *m_pSkyboxAssetRegistry; }
		systems::MaterialLibraryRegistry& GetMaterialLibraryRegistry() { return *m_pMaterialLibraryRegistry; }
		systems::ThumbnailManager& GetThumbnailManager() { return *m_pThumbnailManger; }
		systems::TextureGenerationManager& GetTextureGenerationManager() { return *m_pTextureGenerationManager; }
		systems::ExportManager& GetExportManager() { return *m_pExportManager; }

		graphics::ImageBasedLightingRenderer& GetIblRenderer() { return *m_pIbl_renderer; }
		graphics::Renderer& GetRenderer() { return *m_pRenderer; }
		graphics::Scene& GetScene() { return *m_pScene; }
		graphics::ThumbnailScene& GetThumbnailScene() { return *m_pThumbnailScene; }


		gui::UserInterface& GetUserInterface() { return *m_pUserInterface; }

	private:
		bool m_isInitialized = false;
		
		core::Window* m_pWindow = nullptr;
		
		std::unique_ptr<core::Logger> m_pLogger;
		std::unique_ptr<core::Clock> m_pClock;
		std::unique_ptr<core::FileDirectories> m_pFileDirectories;
		std::unique_ptr<core::DropHandler> m_pDropHandler;
		


		std::unique_ptr<systems::InputSystem> m_pInputSystem;
		std::unique_ptr<systems::SkyboxAssetRegistry> m_pSkyboxAssetRegistry;		
		std::unique_ptr<systems::MaterialLibraryRegistry> m_pMaterialLibraryRegistry;
		std::unique_ptr<systems::ThumbnailManager> m_pThumbnailManger;
		std::unique_ptr<systems::TextureGenerationManager> m_pTextureGenerationManager;
		std::unique_ptr<systems::ExportManager> m_pExportManager;
		


		std::unique_ptr<graphics::ImageBasedLightingRenderer> m_pIbl_renderer;
		std::unique_ptr<graphics::Renderer> m_pRenderer;
		std::unique_ptr<graphics::Scene> m_pScene;
		std::unique_ptr<graphics::ThumbnailScene> m_pThumbnailScene;

		std::unique_ptr<gui::UserInterface> m_pUserInterface;
		


	};


}


#endif // !MNEMOSY_ENGINE_H

