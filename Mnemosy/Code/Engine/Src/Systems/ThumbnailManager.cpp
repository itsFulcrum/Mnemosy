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
	}

	void ThumbnailManager::Update() {

		// When we want to refresh a thumbnail we need to delete the current texture first and then create a new one.  
		// But we cant delete it emediatly because we already gave its texture id to im gui for rendering at the end of the frame. 
		// so we have to delay the refresh to the next frame and update before we set up the next imGuiFrame

		if (!m_thumbnailsQuedForRefresh.empty()) {

			for (int i = 0; i < m_thumbnailsQuedForRefresh.size(); i++) {

				glDeleteTextures(1, &m_thumbnailsQuedForRefresh[i]->thumbnailTexure_ID);
				//MNEMOSY_DEBUG("Delete GL TEX ID {}", thumbnailQuedForRefresh[i].thumbnailTexure_ID);
				m_thumbnailsQuedForRefresh[i]->thumbnailLoaded = false;
				m_thumbnailsQuedForRefresh[i]->thumbnailTexure_ID = 0;
			}

			m_thumbnailsQuedForRefresh.clear();
			m_activeFolderFullyLoaded = false;
		}

	}

	void ThumbnailManager::RenderThumbnailOfActiveMaterial(fs::path& pathToThumbnail, FolderNode* selectedFolder,unsigned int activeMaterialID) {


		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();

		fs::path thumbnailPath = fs::path(activeMat.Name + "_thumbnail.ktx2");
		fs::path thumbnailAbsolutePath = pathToThumbnail;

		graphics::Renderer& renderer = MnemosyEngine::GetInstance().GetRenderer();
		renderer.RenderThumbnail(activeMat);

		graphics::KtxImage thumbnailKtx;
		thumbnailKtx.ExportGlTexture(thumbnailAbsolutePath.generic_string().c_str(), renderer.GetThumbnailRenderTextureID(), 3, renderer.GetThumbnailResolution(), renderer.GetThumbnailResolution(), graphics::ktxImgFormat::MNSY_COLOR, false);

		// check if the active material is part of the currently opend folder and refresh if needed
		if (selectedFolder->HasMaterials()) {

			for (int i = 0; i < selectedFolder->subMaterials.size(); i++) {

				if (selectedFolder->subMaterials[i].runtime_ID == activeMaterialID) {

					m_thumbnailsQuedForRefresh.push_back(&selectedFolder->subMaterials[i]);
					break;
				}
			}
		}



	}
	//void ThumbnailManager::RefreshThumbnail(MaterialInfo& materialInfo,fs::path& pathToThumbnail) {
	//	/// NOT IN USE AT THE MOMENT...
	//	glDeleteTextures(1, &materialInfo.thumbnailTexure_ID);
	//	MNEMOSY_DEBUG("Delete GL TEX ID {}", materialInfo.thumbnailTexure_ID);
	//	materialInfo.thumbnailTexure_ID = 0;
	//	materialInfo.thumbnailLoaded = false;


	//	m_activeFolderFullyLoaded = false;
	//	//glGenTextures(1, &materialInfo.thumbnailTexure_ID);
	//	//graphics::KtxImage thumbnailImg;
	//	//thumbnailImg.LoadKtx(pathToThumbnail.generic_string().c_str(), materialInfo.thumbnailTexure_ID);
	//}
	
	void ThumbnailManager::DeleteThumbnailFromCache(MaterialInfo& materialInfo) {
		glDeleteTextures(1, &materialInfo.thumbnailTexure_ID);
		materialInfo.thumbnailTexure_ID = 0;
		materialInfo.thumbnailLoaded = false;
		//MNEMOSY_DEBUG("Deleted thumbnail: {}, from memory", materialInfo.name);

	}


	void ThumbnailManager::DeleteLoadedThumbnailsOfActiveFolder(FolderNode* activeFolder)  {

		if (activeFolder == nullptr)
			return;

		// delete currently loaded thumbnails
		if (activeFolder->HasMaterials()) {

			for (int i = 0; i < activeFolder->subMaterials.size(); i++) {

				if (activeFolder->subMaterials[i].thumbnailLoaded) {
					//MNEMOSY_DEBUG("Delete GL TEX ID {}", activeFolder->subMaterials[i].thumbnailTexure_ID);
					glDeleteTextures(1, &activeFolder->subMaterials[i].thumbnailTexure_ID);
					activeFolder->subMaterials[i].thumbnailLoaded = false;
					activeFolder->subMaterials[i].thumbnailTexure_ID = 0;
				}
			}
		}

		m_activeFolderFullyLoaded = false;

	}

	void ThumbnailManager::LoadThumbnailsOfActiveFolder(FolderNode* activeFolder, fs::path folderDirectory) {

		if (activeFolder == nullptr)
			return;

		if (m_activeFolderFullyLoaded)
			return;

		for (int i = 0; i < activeFolder->subMaterials.size(); i++) {

			if (!activeFolder->subMaterials[i].thumbnailLoaded) {

				//glGenTextures(1, &activeFolder->subMaterials[i].thumbnailTexure_ID);

				fs::path thumbnailPath = folderDirectory / fs::path(activeFolder->subMaterials[i].name) / fs::path(activeFolder->subMaterials[i].name + "_thumbnail.ktx2");

				fs::directory_entry thumbnailFile = fs::directory_entry(thumbnailPath);
				if (thumbnailFile.exists()) {

					graphics::KtxImage ktxThumbnail;
					bool success = ktxThumbnail.LoadKtx(thumbnailPath.generic_string().c_str(), activeFolder->subMaterials[i].thumbnailTexure_ID);
					if (success) {
						activeFolder->subMaterials[i].thumbnailLoaded = true;
					}
					else {
						MNEMOSY_WARN("Loaded thumbnail FAILED: {}", activeFolder->subMaterials[i].name);
						glDeleteTextures(1, &activeFolder->subMaterials[i].thumbnailTexure_ID);
						return;

					}

				}
				else {
					MNEMOSY_WARN("Loaded thumbnail FAILED: {}, thumbnail file does not exist at {}", activeFolder->subMaterials[i].name,thumbnailPath.generic_string());
				}


				//MNEMOSY_DEBUG("Loaded Thumbnail for {} GLTex ID: {}", activeFolder->subMaterials[i].name,activeFolder->subMaterials[i].thumbnailTexure_ID);


				if(i == activeFolder->subMaterials.size()){
					m_activeFolderFullyLoaded = true;
					MNEMOSY_TRACE("Thumbnails fully loaded");
				}

				// When calling this functions each frame we should break here to only load one thumbnail at a time
				break;
			}
		}
	}




} // ! mnemosy::systems