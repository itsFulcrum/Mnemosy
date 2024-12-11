#ifndef SKYBOX_ASSET_REGISTRY_H
#define SKYBOX_ASSET_REGISTRY_H

#include <string>
#include <vector>


namespace mnemosy::systems {
	struct LibEntry;
}

namespace mnemosy::graphics {
	class Skybox;
}

namespace mnemosy::systems
{
	class SkyboxAssetRegistry
	{
	public:
		SkyboxAssetRegistry()  = default;
		~SkyboxAssetRegistry() = default;

		void Init();
		void Shutdown();

		graphics::Skybox* LoadPreviewSkybox(const uint16_t id, const bool setAsSelected);

		void AddLibEntryToPreviewSkyboxes(systems::LibEntry* libEntry);

		void RemoveEntry(const uint16_t id);

		const std::vector<std::string>& GetEntryList() { return m_entryNames; }
		uint16_t GetCurrentSelectedID() { return m_currentSelected; }
		uint16_t GetLastSessionSelectedEntryID() { return m_lastSelectedEntry; }

	private:
		void LoadDataFile();
		void SaveDataFile();

		void RemoveFilesForEntry(const std::string& name);

		std::vector<std::string> m_entryNames;

		uint16_t m_currentSelected = 0;

		uint16_t m_lastSelectedEntry = 0; // refers to the last selected that was stored when the program was closed.
		
	};


} // !mnemosy::systems
#endif // !SKYBOX_ASSET_REGISTRY_H
