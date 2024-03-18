#include "Include/Systems/ThumbnailManager.h"

#include "Include/Core/Log.h"
#include "Include/MnemosyEngine.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Graphics/Scene.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Graphics/Utils/KtxImage.h"
#include "Include/Graphics/Material.h"

#include <glad/glad.h>



namespace mnemosy::systems {

	ThumbnailManager::ThumbnailManager() {
	
	
	}

	ThumbnailManager::~ThumbnailManager() {
	
	
	}

	void ThumbnailManager::RenderThumbnailOfActiveMaterial(fs::path& pathToThumbnail, FolderNode* selectedFolder,unsigned int activeMaterialID) {


		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();

		fs::path thumbnailPath = fs::path(activeMat.Name + "_thumbnail.ktx2");
		fs::path thumbnailAbsolutePath = pathToThumbnail;

		graphics::Renderer& renderer = MnemosyEngine::GetInstance().GetRenderer();
		renderer.RenderThumbnail(activeMat);

		graphics::KtxImage thumbnailKtx;
		thumbnailKtx.ExportGlTexture(thumbnailAbsolutePath.generic_string().c_str(), renderer.GetRenderTextureId(), 3, renderer.GetThumbnailResolution(), renderer.GetThumbnailResolution(), graphics::ktxImgFormat::MNSY_COLOR, false);

		// check if the active material is part of the currently opend folder and refresh if needed
		if (selectedFolder->HasMaterials()) {
			for (systems::MaterialInfo mat : selectedFolder->subMaterials) {

				if (mat.runtime_ID == activeMaterialID) {

					RefreshThumbnail(mat, thumbnailAbsolutePath);
					break;
				}
			}
		}



	}
	void ThumbnailManager::RefreshThumbnail(MaterialInfo& materialInfo,fs::path& pathToThumbnail) {

		glDeleteTextures(1, &materialInfo.thumbnailTexure_ID);
		materialInfo.thumbnailTexure_ID = 0;
		glGenTextures(1, &materialInfo.thumbnailTexure_ID);

		graphics::KtxImage thumbnailImg;
		thumbnailImg.LoadKtx(pathToThumbnail.generic_string().c_str(), materialInfo.thumbnailTexure_ID);
	}
	void ThumbnailManager::DeleteThumbnailFromCache(MaterialInfo& materialInfo) {
		glDeleteTextures(1, &materialInfo.thumbnailTexure_ID);
		materialInfo.thumbnailTexure_ID = 0;
		materialInfo.thumbnailLoaded = false;
		MNEMOSY_TRACE("Deleted thumbnail: {}, from memory", materialInfo.name);

	}


	void ThumbnailManager::DeleteLoadedThumbnailsOfActiveFolder(FolderNode* activeFolder)  {

		if (activeFolder == nullptr)
			return;

		// delete currently loaded thumbnails
		if (activeFolder->HasMaterials()) {

			for (int i = 0; i < activeFolder->subMaterials.size(); i++) {

				if (activeFolder->subMaterials[i].thumbnailLoaded) {
					glDeleteTextures(1, &activeFolder->subMaterials[i].thumbnailTexure_ID);
					activeFolder->subMaterials[i].thumbnailLoaded = false;
					activeFolder->subMaterials[i].thumbnailTexure_ID = 0;
					MNEMOSY_TRACE("Deleted thumbnail for {}", activeFolder->subMaterials[i].name);
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

				glGenTextures(1, &activeFolder->subMaterials[i].thumbnailTexure_ID);

				fs::path thumbnailPath = folderDirectory / fs::path(activeFolder->subMaterials[i].name) / fs::path(activeFolder->subMaterials[i].name + "_thumbnail.ktx2");

				graphics::KtxImage ktxThumbnail;
				bool success = ktxThumbnail.LoadKtx(thumbnailPath.generic_string().c_str(), activeFolder->subMaterials[i].thumbnailTexure_ID);
				if (!success) {

					MNEMOSY_TRACE("Loaded thumbnail FAILED: {}", activeFolder->subMaterials[i].name);
				}

				activeFolder->subMaterials[i].thumbnailLoaded = true;
				MNEMOSY_TRACE("Loaded thumbnail for {}", activeFolder->subMaterials[i].name);


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