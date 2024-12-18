#ifndef THUMBNAIL_MANAGER_H
#define THUMBNAIL_MANAGER_H

#include <filesystem>
#include <vector>


namespace mnemosy::systems {
	struct LibEntry;
	struct FolderNode;
}
namespace mnemosy::graphics {
	class PbrMaterial;
}

namespace mnemosy::systems {


	class ThumbnailManager {
	public:
		ThumbnailManager()  = default;
		~ThumbnailManager() = default;
		
		void Init();
		void Shutdown();

		void Update();
				
		void RenderThumbnailForActiveLibEntry(LibEntry* activeLibEntry);


		void AddLibEntryToActiveThumbnails(LibEntry* libEntry);
		void RemoveLibEntryFromActiveThumbnails(LibEntry* libEntry);

		void UnloadAllThumbnails();

		void RenderThumbnailForAnyLibEntry_Slow_Fallback(LibEntry* libEntry);
	private:

		void DeleteThumbnailGLTexture_Internal(LibEntry* libEntry);
		void LoadThumbnailForMaterial_Internal(LibEntry* libEntry);


		std::vector<systems::LibEntry*> m_thumbnailsQuedForRefresh;		
		std::vector<systems::LibEntry*> m_activeEntries;
		bool m_activeEntriesFullyLoaded = false;
	
	};
} // ! mnemosy::systems
#endif // !THUMBNAIL_MANAGER_H
