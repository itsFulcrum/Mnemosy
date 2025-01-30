#include "Include/Systems/ThumbnailManager.h"

#include "Include/Core/Log.h"
#include "Include/MnemosyEngine.h"
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/LibraryProcedures.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Graphics/Scene.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Graphics/Utils/KtxImage.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/ThumbnailScene.h"
#include "Include/Graphics/Camera.h"

#include <glad/glad.h>



namespace mnemosy::systems {

	void ThumbnailManager::Init() {
		m_activeEntriesFullyLoaded = false;
	}

	void ThumbnailManager::Shutdown() {
		if (!m_thumbnailsQuedForRefresh.empty()) {

			m_thumbnailsQuedForRefresh.clear();
		}

		UnloadAllThumbnails();
	}

	void ThumbnailManager::Update() {

		// When we want to refresh a thumbnail we need to delete the current texture first and then create a new one.  
		// But we cant delete it emediatly because we already gave its texture id to im gui for rendering at the end of the frame. 
		// so we have to delay the refresh to the next frame and update before we set up the next imGuiFrame

		if (!m_thumbnailsQuedForRefresh.empty()) {

			for (int i = 0; i < m_thumbnailsQuedForRefresh.size(); i++) {

				DeleteThumbnailGLTexture_Internal(m_thumbnailsQuedForRefresh[i]);
			}

			m_thumbnailsQuedForRefresh.clear();
			m_activeEntriesFullyLoaded = false;
		}
		
		//  Loading thumbnails
		if (m_activeEntries.empty())
			return;

		if (m_activeEntriesFullyLoaded)
			return;

		
		for (int i = 0; i < m_activeEntries.size(); i++) {

			if (!m_activeEntries[i]->thumbnailLoaded) {

				LoadThumbnailForMaterial_Internal(m_activeEntries[i]);
				m_activeEntriesFullyLoaded = false;
				return;
			}
		}

		// if we reach this code it means all thumbnails in the list where loaded
		
		m_activeEntriesFullyLoaded = true;

	}

	// TODO: handle entry types
	void ThumbnailManager::RenderThumbnailForActiveLibEntry(LibEntry* libEntry) {

		namespace fs = std::filesystem;

		fs::path thumbnailAbsolutePath = systems::LibProcedures::LibEntry_GetFolderPath(libEntry) / fs::u8path(libEntry->name + "_thumbnail.ktx2");


		graphics::Renderer& renderer = MnemosyEngine::GetInstance().GetRenderer();



		if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {

			graphics::PbrMaterial& activePbrMat = MnemosyEngine::GetInstance().GetScene().GetPbrMaterial();

			renderer.RenderThumbnail_PbrMaterial(activePbrMat);
			
		}
		else if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {

			graphics::UnlitMaterial* unlitMat = MnemosyEngine::GetInstance().GetScene().GetUnlitMaterial();

			renderer.RenderThumbnail_UnlitMaterial(unlitMat);
		}
		else if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

			graphics::Skybox& skybox = MnemosyEngine::GetInstance().GetScene().GetSkybox();
			
			

			renderer.RenderThumbnail_SkyboxMaterial(skybox);
		}

		// export rendered thumbnail

		graphics::KtxImage thumbnailKtx;

		int thumbnailRes = renderer.GetThumbnailResolutionValue(renderer.GetThumbnailResolutionEnum());
		thumbnailKtx.ExportGlTexture(thumbnailAbsolutePath.generic_string().c_str(), renderer.GetThumbnailRenderTextureID(), 3, thumbnailRes, thumbnailRes, graphics::ktxImgFormat::MNSY_COLOR, false);


		// check if the thumbnail is currently loaded and then que it for refresh
		if (!m_activeEntries.empty()) {

			for (int i = 0; i < m_activeEntries.size(); i++) {
				
				if (m_activeEntries[i]->runtime_ID == libEntry->runtime_ID) {
					
					m_thumbnailsQuedForRefresh.push_back(m_activeEntries[i]);
					break;
				}
			}
		}

	}

	void ThumbnailManager::RenderThumbnailForAnyLibEntry_Slow_Fallback(LibEntry* libEntry) {

		MNEMOSY_ASSERT(libEntry != nullptr, "This should not happen");

		fs::path entryFolder  = systems::LibProcedures::LibEntry_GetFolderPath(libEntry);



		fs::path thumbnailPath = entryFolder / fs::path(libEntry->name + "_thumbnail.ktx2");
		graphics::Renderer& renderer = MnemosyEngine::GetInstance().GetRenderer();

		if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {


			graphics::PbrMaterial* pbrMat = systems::LibProcedures::LibEntry_PbrMaterial_LoadFromFile_Multithreaded(libEntry,true);

			renderer.RenderThumbnail_PbrMaterial(*pbrMat);
			delete pbrMat;
		}
		else if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {

			graphics::UnlitMaterial* unlitMat = systems::LibProcedures::LibEntry_UnlitMaterial_LoadFromFile(libEntry,true);

			renderer.RenderThumbnail_UnlitMaterial(unlitMat);

			delete unlitMat;
		}
		else if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

			fs::path entryFolder = systems::LibProcedures::LibEntry_GetFolderPath(libEntry);

			graphics::Skybox* sky = systems::LibProcedures::LibEntry_SkyboxMaterial_LoadFromFile(entryFolder,libEntry->name, true);

			renderer.RenderThumbnail_SkyboxMaterial(*sky);

			delete sky;
		}

		// export ktx image
		graphics::KtxImage thumbnailKtx;
		int thumbnailRes = renderer.GetThumbnailResolutionValue(renderer.GetThumbnailResolutionEnum());
		thumbnailKtx.ExportGlTexture(thumbnailPath.generic_string().c_str(), renderer.GetThumbnailRenderTextureID(), 3, thumbnailRes, thumbnailRes, graphics::ktxImgFormat::MNSY_COLOR, false);


		// check if the thumbnail is currently loaded and then que it for refresh
		if (!m_activeEntries.empty()) {

			for (int i = 0; i < m_activeEntries.size(); i++) {

				if (m_activeEntries[i]->runtime_ID == libEntry->runtime_ID) {

					m_thumbnailsQuedForRefresh.push_back(m_activeEntries[i]);
					break;
				}
			}
		}
	}

	void ThumbnailManager::AddLibEntryToActiveThumbnails(LibEntry* libEntry) {

		MNEMOSY_ASSERT(libEntry != nullptr, "NO!");
		

		// check if its already in the list
		if (!m_activeEntries.empty()) {

			for (int i = 0; i < m_activeEntries.size(); i++) {

				if (m_activeEntries[i]->runtime_ID == libEntry->runtime_ID) {
					MNEMOSY_ERROR("ThumbnailManager::AddMaterialForThumbnailing: Entry is already in the thumbnail list");
					return;
				}
			}
		}

		m_activeEntries.push_back(libEntry);

		m_activeEntriesFullyLoaded = false;
	}

	void ThumbnailManager::RemoveLibEntryFromActiveThumbnails(LibEntry* libEntry) {

		if (m_activeEntries.empty())
			return;


		MNEMOSY_ASSERT(libEntry != nullptr, "Should not happen");

		for (int i = 0; i < m_activeEntries.size(); i++) {

			if (m_activeEntries[i]->runtime_ID == libEntry->runtime_ID) {

				DeleteThumbnailGLTexture_Internal(libEntry);
				m_activeEntries.erase(m_activeEntries.begin() + i);
				return;
			}
		}


	}

	void ThumbnailManager::UnloadAllThumbnails() {


		if (m_activeEntries.empty())
			return;

		for (int i = 0; i < m_activeEntries.size(); i++) {

			DeleteThumbnailGLTexture_Internal(m_activeEntries[i]);
		}

		m_activeEntries.clear();
		m_activeEntriesFullyLoaded = true;
	}

	// Delete the gl texture of the thumbnail
	void ThumbnailManager::DeleteThumbnailGLTexture_Internal(LibEntry* libEntry) {

		MNEMOSY_ASSERT(libEntry != nullptr, "We should make sure to unload all thumbnails first");

		glDeleteTextures(1, &libEntry->thumbnailTexure_ID);
		libEntry->thumbnailLoaded = false;
		libEntry->thumbnailTexure_ID = 0;
	}

	void ThumbnailManager::LoadThumbnailForMaterial_Internal(LibEntry* libEntry) {

		namespace fs = std::filesystem;

		fs::path libPath = MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().ActiveLibCollection_GetFolderPath();

		fs::path materialFolderPath = libPath / libEntry->GetPathFromRoot();// libEntry->parent->pathFromRoot / fs::path(libEntry->name);

		fs::path thumbnailPath = materialFolderPath / fs::path(libEntry->name + "_thumbnail.ktx2");

		if (fs::exists(thumbnailPath)) {

			graphics::KtxImage ktxThumbnail;
			bool success = ktxThumbnail.LoadKtx(thumbnailPath.generic_string().c_str(), libEntry->thumbnailTexure_ID);
			if (success) {
				libEntry->thumbnailLoaded = true;
			}
			else {
				MNEMOSY_WARN("Failed to load thumbnail for material: {}", libEntry->name);
				glDeleteTextures(1, &libEntry->thumbnailTexure_ID);
				return;

			}
		}
		else {
			MNEMOSY_WARN("Failed to load thumbnail, Generating new: {}",  thumbnailPath.generic_string());

			// this is potentially super slow because we have to load all textures and stuff of the material but should work and generate a new thumbnail
			RenderThumbnailForAnyLibEntry_Slow_Fallback(libEntry);
		}

	}


} // ! mnemosy::systems