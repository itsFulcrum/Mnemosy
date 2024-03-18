#ifndef THUMBNAIL_MANAGER_H
#define THUMBNAIL_MANAGER_H

#include <filesystem>

namespace fs = std::filesystem;

namespace mnemosy::systems {
	struct MaterialInfo;
	struct FolderNode;
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

	private:
		bool m_activeFolderFullyLoaded = false;
	};
} // ! mnemosy::systems
#endif // !THUMBNAIL_MANAGER_H
