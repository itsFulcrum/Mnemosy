#include "Include/Systems/SkyboxAssetRegistry.h"

#include "Include/Core/Log.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"

#include <fstream>


namespace mnemosy::systems
{
	// Public Methods

	SkyboxAssetRegistry::SkyboxAssetRegistry()
	{

		mnemosy::core::FileDirectories& FD = MnemosyEngine::GetInstance().GetFileDirectories();

		m_dataFileName = "SkyboxAssetsRegistry.mnsydata";
		m_pathToDatafile = FD.GetDataPath().generic_string() + "/" + m_dataFileName;

		LoadEntriesFromSavedData();
	}

	SkyboxAssetRegistry::~SkyboxAssetRegistry()
	{
		SaveAllEntriesToDataFile();
	}

	bool SkyboxAssetRegistry::CheckIfExists(std::string name)
	{
		if (m_orderedEntryNames.empty())
			return false;

		for (std::string& entryName : m_orderedEntryNames)
		{
			if (entryName == name)
			{
				return true;
			}
		}

		return false;
	}

	void SkyboxAssetRegistry::AddEntry(std::string name)
	{
		// check if it already exists

		if (CheckIfExists(name))
			return;

		SkyboxAssetEntry newEntry;
		newEntry.skyName			= name;
		newEntry.colorCubeFile		= name + "_cubeColor.ktx2";
		newEntry.irradianceCubeFile	= name + "_cubeIrradiance.ktx2";
		newEntry.prefilterCubeFile	= name + "_cubePrefilter.ktx2";

		m_skyboxAssets.push_back(newEntry);
		m_orderedEntryNames.push_back(name);


		// save to file
		SaveAllEntriesToDataFile();
	}

	void SkyboxAssetRegistry::RemoveEntry(std::string name)
	{

		for (int i = 0; i < m_skyboxAssets.size(); i++)
		{
			if (m_skyboxAssets[i].skyName == name)
			{
				m_skyboxAssets.erase(m_skyboxAssets.begin() + i);
				m_orderedEntryNames.erase(m_orderedEntryNames.begin() + i);
				break;
			}
		}

		SaveAllEntriesToDataFile();
	}

	SkyboxAssetEntry SkyboxAssetRegistry::GetEntry(std::string name)
	{
		SkyboxAssetEntry returnEntry;

		if (!m_skyboxAssets.empty())
		{
			for (SkyboxAssetEntry& entry : m_skyboxAssets)
			{
				if (entry.skyName == name)
				{

					returnEntry.skyName				= entry.skyName;
					returnEntry.colorCubeFile		= entry.colorCubeFile;
					returnEntry.irradianceCubeFile	= entry.irradianceCubeFile;
					returnEntry.prefilterCubeFile	= entry.prefilterCubeFile;

					return returnEntry;
				}
			}
		}

		returnEntry.skyName				= "EntryDoesNotExist";
		returnEntry.colorCubeFile		= "EntryDoesNotExist";
		returnEntry.irradianceCubeFile	= "EntryDoesNotExist";
		returnEntry.prefilterCubeFile	= "EntryDoesNotExist";

		return returnEntry;
	}

	std::vector<std::string>& SkyboxAssetRegistry::GetVectorOfNames()
	{
		return m_orderedEntryNames;
	}

	// Private Methods
	void SkyboxAssetRegistry::LoadEntriesFromSavedData()
	{
		//MNEMOSY_TRACE("LOADING FILE..")
		std::filesystem::directory_entry dataFile = std::filesystem::directory_entry(m_pathToDatafile);
		if (!CheckDataFile(dataFile))
			return;

		std::ifstream dataFileStream;
		dataFileStream.open(m_pathToDatafile);

		nlohmann::json readFile;
		try {
			readFile = nlohmann::json::parse(dataFileStream);
		}
		catch (nlohmann::json::parse_error err) {
			MNEMOSY_ERROR("SkyboxAssetRegistry::LoadEntriesFromSavedData: Error Parsing File. Message: {}", err.what());
			return;
		}

		int amountOfEntries = readFile["2_Header_Info"]["entriesAmount"].get<int>();
		std::vector<std::string> entryNames = readFile["2_Header_Info"]["entryNames"].get<std::vector<std::string>>();
		
		// just in case we ever want to load from file while app is already running clear vectors first
		m_skyboxAssets.clear();
		m_orderedEntryNames.clear();
		for (int i = 0; i < entryNames.size(); i++) 
		{
			AddEntry(entryNames[i]);
		}

		entryNames.clear();
		dataFileStream.close();
	}

	void SkyboxAssetRegistry::SaveAllEntriesToDataFile()
	{
		std::filesystem::directory_entry dataFile = std::filesystem::directory_entry(m_pathToDatafile);
		CheckDataFile(dataFile);

		std::ofstream dataFileStream;
		// clear file first // idk this seems kinda stupid and dangerous but also it gets mees up when just overwriting it // maybe do backup copy first?
		dataFileStream.open(m_pathToDatafile);
		dataFileStream << "";
		dataFileStream.close();
		
		// start Saving
		dataFileStream.open(m_pathToDatafile);
		nlohmann::json SkyboxAssetsJson; // top level json object
		SkyboxAssetsJson["1_Mnemosy_Data_File"] = "SkyboxRegistryData";
		
		nlohmann::json HeaderInfo;
		HeaderInfo["entriesAmount"] = m_orderedEntryNames.size();
		HeaderInfo["entryNames"] = m_orderedEntryNames;
		
		SkyboxAssetsJson["2_Header_Info"] = HeaderInfo;
		
		// add each entry to json object "Asset_Entries"
		nlohmann::json assetEntries;
		for (int i = 0; i < m_skyboxAssets.size(); i++)
		{
			assetEntries[m_skyboxAssets[i].skyName] = m_skyboxAssets[i];
		}
		SkyboxAssetsJson["Asset_Entries"] = assetEntries;


		if(prettyPrintDataFile)
			dataFileStream << SkyboxAssetsJson.dump(2);
		else
			dataFileStream << SkyboxAssetsJson.dump(-1);

		dataFileStream.close();
	}

	void SkyboxAssetRegistry::UpdateOrderedEntryNamesVector()
	{
		//  curently its optimised to directly do it when also loading and saving but keeping function around for later maybe
		m_orderedEntryNames.clear();
		for (int i = 0; i < m_skyboxAssets.size(); i++)
		{
			m_orderedEntryNames.push_back(m_skyboxAssets[i].skyName);
		}
	}

	bool SkyboxAssetRegistry::CheckDataFile(std::filesystem::directory_entry dataFile)
	{
		if (!dataFile.exists())
		{
			MNEMOSY_ERROR("SkyboxAssetRegistry::CheckDataFile: File did Not Exist: {} \n Creating new file at that location", m_pathToDatafile);
			std::ofstream file;
			file.open(m_pathToDatafile);
			file << "";
			file.close();
			return false;
		}
		if (!dataFile.is_regular_file())
		{
			MNEMOSY_ERROR("SkyboxAssetRegistry::CheckDataFile: File is not a regular file: {}", m_pathToDatafile);
			// maybe need to delete unregular file first idk should never happen anyhow
			std::ofstream file;
			file.open(m_pathToDatafile);
			file << "";
			file.close();
			return false;
		}

		return true;
	}

} // !mnemosy::systems