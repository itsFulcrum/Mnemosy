#ifndef THUMBNAIL_MANAGER_H
#define THUMBNAIL_MANAGER_H

#include <filesystem>
#include <vector>


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
		
		void Update();
				
		void RenderThumbnailOfActiveMaterial(std::filesystem::path& pathToThumbnail,FolderNode* selectedFolder,unsigned int activeMaterialID);

		void AddMaterialForThumbnailing(MaterialInfo* material);
		void RemoveMaterialFromThumbnailing(MaterialInfo* material);

		void UnloadAllThumbnails();

	private:
		void DeleteThumbnailGLTexture_Internal(MaterialInfo* material);
		void LoadThumbnailForMaterial_Internal(MaterialInfo* material);

		bool m_activeMaterialsFullyLoaded = false;

		std::vector<systems::MaterialInfo*> m_thumbnailsQuedForRefresh;
		
		std::vector<systems::MaterialInfo*> m_activeMaterials;
	
	};
} // ! mnemosy::systems
#endif // !THUMBNAIL_MANAGER_H
