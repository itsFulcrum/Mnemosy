#include "Include/Systems/ThumbnailManager.h"

#include "Include/Core/Log.h"
#include "Include/MnemosyEngine.h"
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

	ThumbnailManager::ThumbnailManager() {
	
	}

	ThumbnailManager::~ThumbnailManager() {
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
			m_activeMaterialsFullyLoaded = false;
		}
		
		//  Loading thumbnails
		if (m_activeMaterials.empty())
			return;

		if (m_activeMaterialsFullyLoaded)
			return;

		
		for (int i = 0; i < m_activeMaterials.size(); i++) {

			if (!m_activeMaterials[i]->thumbnailLoaded) {

				LoadThumbnailForMaterial_Internal(m_activeMaterials[i]);
				m_activeMaterialsFullyLoaded = false;
				return;
			}
		}

		// if we reach this code it means all thumbnails in the list where loaded
		
		m_activeMaterialsFullyLoaded = true;

	}

	void ThumbnailManager::RenderThumbnailOfActiveMaterial(std::filesystem::path& pathToThumbnail, FolderNode* selectedFolder,unsigned int activeMaterialID) {

		namespace fs = std::filesystem;

		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();

		fs::path thumbnailPath = fs::path(activeMat.Name + "_thumbnail.ktx2");
		fs::path thumbnailAbsolutePath = pathToThumbnail;

		graphics::Renderer& renderer = MnemosyEngine::GetInstance().GetRenderer();
		renderer.RenderThumbnail(activeMat);

		graphics::KtxImage thumbnailKtx;
		thumbnailKtx.ExportGlTexture(thumbnailAbsolutePath.generic_string().c_str(), renderer.GetThumbnailRenderTextureID(), 3, renderer.GetThumbnailResolution(), renderer.GetThumbnailResolution(), graphics::ktxImgFormat::MNSY_COLOR, false);

		// check if the active material is part of the active materials
		if (!m_activeMaterials.empty()) {

			for (int i = 0; i < m_activeMaterials.size(); i++) {
				
				if (m_activeMaterials[i]->runtime_ID == activeMaterialID) {
					
					m_thumbnailsQuedForRefresh.push_back(m_activeMaterials[i]);
					break;
				}
			}
		}

	}

	void ThumbnailManager::AddMaterialForThumbnailing(MaterialInfo* material) {

		MNEMOSY_ASSERT(material != nullptr,"NO!")
		

		// check if its already in the list
		if (!m_activeMaterials.empty()) {

			for (int i = 0; i < m_activeMaterials.size(); i++) {

				if (m_activeMaterials[i]->runtime_ID == material->runtime_ID) {
					MNEMOSY_ERROR("ThumbnailManager::AddMaterialForThumbnailing: Material is already in the thumbnail list");
					return;
				}
			}
		}

		m_activeMaterials.push_back(material);

		m_activeMaterialsFullyLoaded = false;
	}

	void ThumbnailManager::RemoveMaterialFromThumbnailing(MaterialInfo* material) {

		if (m_activeMaterials.empty())
			return;


		MNEMOSY_ASSERT(material != nullptr, "Should not happen");

		for (int i = 0; i < m_activeMaterials.size(); i++) {

			if (m_activeMaterials[i]->runtime_ID == material->runtime_ID) {

				DeleteThumbnailGLTexture_Internal(material);
				m_activeMaterials.erase(m_activeMaterials.begin() + i);
				return;
			}
		}


	}

	void ThumbnailManager::UnloadAllThumbnails() {


		if (m_activeMaterials.empty())
			return;

		for (int i = 0; i < m_activeMaterials.size(); i++) {

			DeleteThumbnailGLTexture_Internal(m_activeMaterials[i]);
		}

		m_activeMaterials.clear();
		m_activeMaterialsFullyLoaded = true;
	}

	// Delete the gl texture of the thumbnail
	void ThumbnailManager::DeleteThumbnailGLTexture_Internal(MaterialInfo* material) {

		MNEMOSY_ASSERT(material != nullptr, "We should make sure to unload all thumbnails first");

		glDeleteTextures(1, &material->thumbnailTexure_ID);
		material->thumbnailLoaded = false;
		material->thumbnailTexure_ID = 0;
	}

	void ThumbnailManager::LoadThumbnailForMaterial_Internal(MaterialInfo* material) {

		namespace fs = std::filesystem;

		fs::path libPath = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();

		fs::path materialFolderPath = libPath /  material->parent->pathFromRoot / fs::path(material->name);

		fs::path thumbnailPath = materialFolderPath / fs::path(material->name + "_thumbnail.ktx2");

		fs::directory_entry thumbnailFile = fs::directory_entry(thumbnailPath);
		if (thumbnailFile.exists()) {

			graphics::KtxImage ktxThumbnail;
			bool success = ktxThumbnail.LoadKtx(thumbnailPath.generic_string().c_str(), material->thumbnailTexure_ID);
			if (success) {
				material->thumbnailLoaded = true;
			}
			else {
				MNEMOSY_WARN("Failed to load thumbnail for material: {}", material->name);
				glDeleteTextures(1, &material->thumbnailTexure_ID);
				return;

			}
		}
		else {
			MNEMOSY_WARN("Failed to load thumbnail for material: {}, thumbnail file does not exist at {}", material->name, thumbnailPath.generic_string());
		}

	}


} // ! mnemosy::systems