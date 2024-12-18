#ifndef MNEMOSY_ENGINE_H
#define MNEMOSY_ENGINE_H

#include <memory>
#include "Include/Core/flcrm_arena_alloc.h"

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
	class MeshRegistry;
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

#define arena_malloc(type)	MnemosyEngine::GetInstance().arena_persistent_malloc(sizeof(type),alignof(type))
#define arena_placement_new(type) new (MnemosyEngine::GetInstance().arena_persistent_malloc(sizeof(type),alignof(type))) type 

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


		// Getters to the subsystems
		core::Window& GetWindow()											{ return *m_pWindow; }
		core::FileDirectories& GetFileDirectories()							{ return *m_pFileDirectories; }
		core::Clock& GetClock()												{ return *m_pClock; }
		core::DropHandler& GetDropHandler()									{ return *m_pDropHandler; }
		
		systems::InputSystem& GetInputSystem()								{ return *m_pInputSystem; }
		systems::SkyboxAssetRegistry& GetSkyboxAssetRegistry()				{ return *m_pSkyboxAssetRegistry; }
		systems::MaterialLibraryRegistry& GetMaterialLibraryRegistry()		{ return *m_pMaterialLibraryRegistry; }
		systems::ThumbnailManager& GetThumbnailManager()					{ return *m_pThumbnailManager; }
		systems::TextureGenerationManager& GetTextureGenerationManager()	{ return *m_pTextureGenerationManager; }
		systems::ExportManager& GetExportManager()							{ return *m_pExportManager; }
		systems::MeshRegistry& GetMeshRegistry()							{ return *m_pMeshRegistry; }

		graphics::ImageBasedLightingRenderer& GetIblRenderer() { return *m_pIbl_renderer; }
		graphics::Renderer& GetRenderer() { return *m_pRenderer; }
		graphics::Scene& GetScene() { return *m_pScene; }
		graphics::ThumbnailScene& GetThumbnailScene() { return *m_pThumbnailScene; }

		gui::UserInterface& GetUserInterface() { return *m_pUserInterface; }

		void* arena_persistent_malloc(size_t size, size_t align);

	private:

		flcrm::Arena m_arena_persistent;

		bool m_isInitialized = false;
		


		core::Window* m_pWindow = nullptr;
		
		std::unique_ptr<core::Logger> m_pLogger;
		core::Clock* m_pClock;
		core::FileDirectories* m_pFileDirectories;

		core::DropHandler* m_pDropHandler;
		


		systems::InputSystem* m_pInputSystem;
		systems::SkyboxAssetRegistry* m_pSkyboxAssetRegistry;
		systems::MaterialLibraryRegistry* m_pMaterialLibraryRegistry;
		systems::MeshRegistry* m_pMeshRegistry;

		systems::ThumbnailManager* m_pThumbnailManager;
		systems::TextureGenerationManager* m_pTextureGenerationManager;
		systems::ExportManager* m_pExportManager;
		

		graphics::ImageBasedLightingRenderer* m_pIbl_renderer;
		graphics::Renderer* m_pRenderer;

		graphics::Scene* m_pScene;
		graphics::ThumbnailScene* m_pThumbnailScene;

		gui::UserInterface* m_pUserInterface;
	
	};


}


#endif // !MNEMOSY_ENGINE_H

