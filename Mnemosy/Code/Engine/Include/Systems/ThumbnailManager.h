#ifndef THUMBNAIL_MANAGER_H
#define THUMBNAIL_MANAGER_H

#include <filesystem>

namespace fs = std::filesystem;

namespace mnemosy::systems {
	struct MaterialInfo;
	struct FolderNode;
}
namespace mnemosy::graphics {
	class Material;
}

namespace mnemosy::systems {


	class ThumbnailManager {
	public:
		ThumbnailManager();
		~ThumbnailManager();
		


		
		void RenderThumbnailOfActiveMaterial(fs::path& pathToThumbnail,FolderNode* selectedFolder,unsigned int activeMaterialID);

		void RefreshThumbnail(MaterialInfo& materialInfo, fs::path& pathToThumbnail);

		void DeleteThumbnailFromCache(MaterialInfo& materialInfo);

		void DeleteLoadedThumbnailsOfActiveFolder(FolderNode* activeFolder);
		void LoadThumbnailsOfActiveFolder(FolderNode* activeFolder, fs::path folderDirectory);

		void NewThumbnailInActiveFolder() { m_activeFolderFullyLoaded = false; }


		unsigned int GetThumbnailTextureID() { return m_thumbnailRenderTexture_Id; }
	private:
		bool m_activeFolderFullyLoaded = false;



		void CreateThumbnailFramebuffers();
		void RenderThumbnail(graphics::Material& activeMaterial);


		unsigned int m_fbo = 0;
		unsigned int m_rbo = 0;
		unsigned int m_renderTexture = 0;


		unsigned int m_thumbnailRenderTexture_Id = 0;
		unsigned int m_blitFbo = 0;
		unsigned int m_thumbnailResolution = 256;


	};
} // ! mnemosy::systems
#endif // !THUMBNAIL_MANAGER_H
