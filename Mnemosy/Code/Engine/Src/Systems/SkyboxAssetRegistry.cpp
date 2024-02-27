#include "Include/Systems/SkyboxAssetRegistry.h"

#include "Include/Core/Log.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"

#include <filesystem>
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
				
				
				break;
			}
		}

		// save updated list to file
		// this could be slow consider only doing this on application close or when user explicitly saves
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

					returnEntry.skyName					= entry.skyName;
					returnEntry.colorCubeFile		= entry.colorCubeFile;
					returnEntry.irradianceCubeFile	= entry.irradianceCubeFile;
					returnEntry.prefilterCubeFile	= entry.prefilterCubeFile;

					return returnEntry;
				}
			}
		}

		returnEntry.skyName					= "EntryDoesNotExist";
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

		if (!dataFile.exists())
		{
			MNEMOSY_ERROR("SkyboxAssetRegistry::LoadEntriesFromSavedData: File did Not Exist: {} \n Creating new file at that location", m_pathToDatafile);
			std::ofstream file;
			file.open(m_pathToDatafile);
			file << "";
			file.close();
			return;
		}
		if (!dataFile.is_regular_file())
		{
			MNEMOSY_ERROR("SkyboxAssetRegistry::LoadEntriesFromSavedData: File is not a regular file: {}", m_pathToDatafile);
			return;
		}

		std::ifstream dataFileStream;
		dataFileStream.open(m_pathToDatafile,std::ios::in);

		nlohmann::json readFile;

		try {
			readFile = nlohmann::json::parse(dataFileStream);
			//nlohmann::json readFile{ nlohmann::json::parse(dataFileStream) };
		}
		catch (nlohmann::json::parse_error err) 
		{
			MNEMOSY_ERROR("SkyboxAssetRegistry::LoadEntriesFromSavedData: Error Parsing File. Message: {}", err.what());
			return;
		}

		int amountOfEntries = readFile["2_Header_Info"]["entriesAmount"].get<int>();
		std::vector<std::string> entryNames = readFile["2_Header_Info"]["entryNames"].get<std::vector<std::string>>();


		m_orderedEntryNames.clear();

		for (std::string& name : entryNames)
		{
			AddEntry(name);
		}

		dataFileStream.close();

		UpdateOrderedEntryNamesVector();
	}


	void SkyboxAssetRegistry::SaveAllEntriesToDataFile()
	{
		std::filesystem::directory_entry dataFile = std::filesystem::directory_entry(m_pathToDatafile);

		if (!dataFile.exists())
		{
			MNEMOSY_ERROR("SkyboxAssetRegistry::SaveEntriesToDataFile: File did Not Exist: {} \n Creating new file at that location", m_pathToDatafile);
			std::ofstream file;
			file.open(m_pathToDatafile);
			file << "";
			file.close();
			return;
		}
		if (!dataFile.is_regular_file())
		{
			MNEMOSY_ERROR("SkyboxAssetRegistry::SaveEntriesToDataFile: File is not a regular file: {}", m_pathToDatafile);
			return;
		}

		//std::ifstream()
		//MNEMOSY_WARN("PathToDataFile: {}", m_pathToDatafile);
		
		std::ofstream dataFileStream;
		// clear file first
		dataFileStream.open(m_pathToDatafile);
		dataFileStream << "";
		dataFileStream.close();

		
		dataFileStream.open(m_pathToDatafile);

		std::vector<std::string> allEntryNames;
		
		for (SkyboxAssetEntry& entry : m_skyboxAssets)
		{
			allEntryNames.push_back(entry.skyName);
		}
		
		nlohmann::json SkyboxAssetsJson;
		
		nlohmann::json HeaderInfo;
		HeaderInfo["entriesAmount"] = m_skyboxAssets.size();
		HeaderInfo["entryNames"] = allEntryNames;
		
		SkyboxAssetsJson["1_Mnemosy_Data_File"] = "SkyboxRegistryData";
		SkyboxAssetsJson["2_Header_Info"] = HeaderInfo;
		
		//dataFileStream << HeaderInfo.dump(-1);

		// add each entry
		nlohmann::json assetEntries;
		for (SkyboxAssetEntry& entry : m_skyboxAssets)
		{
			assetEntries[entry.skyName] = entry;

		}

		SkyboxAssetsJson["Asset_Entries"] = assetEntries;

		if(prettyPrintDataFile)
			dataFileStream << SkyboxAssetsJson.dump(2);
		else
			dataFileStream << SkyboxAssetsJson.dump(-1);


		allEntryNames.clear();
		dataFileStream.close();
	
		UpdateOrderedEntryNamesVector();
	}

	void SkyboxAssetRegistry::UpdateOrderedEntryNamesVector()
	{

		m_orderedEntryNames.clear();
		for (int i = 0; i < m_skyboxAssets.size(); i++)
		{
			m_orderedEntryNames.push_back(m_skyboxAssets[i].skyName);
		}

	}





} // !mnemosy::systems