#ifndef SKYBOX_ASSET_REGISTRY_H
#define SKYBOX_ASSET_REGISTRY_H



#include <string>
#include <vector>
#include <filesystem>

#include <nlohmann/json.hpp>

namespace mnemosy::systems
{

	struct SkyboxAssetEntry 
	{
		std::string skyName;
		std::string colorCubeFile;
		std::string irradianceCubeFile;
		std::string prefilterCubeFile;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SkyboxAssetEntry,
		skyName,
		colorCubeFile,
		irradianceCubeFile,
		prefilterCubeFile
		)

	class SkyboxAssetRegistry
	{
	public:
		SkyboxAssetRegistry();
		~SkyboxAssetRegistry();

		bool CheckIfExists(const std::string& name);

		void AddEntry(const std::string& name);
		void RemoveEntry(const std::string& name);

		SkyboxAssetEntry GetEntry(const std::string& name);

		std::vector<std::string>& GetVectorOfNames();



	private:
		void LoadEntriesFromSavedData();
		void SaveAllEntriesToDataFile();
		void UpdateOrderedEntryNamesVector();

		bool CheckDataFile(std::filesystem::directory_entry dataFile);

		std::string m_dataFileName;// = "SkyboxAssetsRegistry.mnsydata";
		std::string m_pathToDatafile;// = "";

		std::vector<SkyboxAssetEntry> m_skyboxAssets;
		std::vector<std::string> m_orderedEntryNames; // ordered just means that they should always be in the same order as the entries are in the m_skyboxAsstes;

		bool prettyPrintDataFile = false;

	};


} // !mnemosy::systems
#endif // !SKYBOX_ASSET_REGISTRY_H
