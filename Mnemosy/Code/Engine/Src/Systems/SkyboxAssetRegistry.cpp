#include "Include/Systems/SkyboxAssetRegistry.h"

#include "Include/Core/Log.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/ExportManager.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/LibraryProcedures.h"
#include "Include/Systems/JsonKeys.h"

#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/Cubemap.h"
#include "Include/Graphics/TextureDefinitions.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Utils/Picture.h"
#include "Include/Graphics/Utils/KtxImage.h"

#include <filesystem>
#include <json.hpp>

namespace mnemosy::systems
{
	void SkyboxAssetRegistry::Init() {

		LoadDataFile();
	}

	void SkyboxAssetRegistry::Shutdown() {

		SaveDataFile();
	}

	graphics::Skybox* SkyboxAssetRegistry::LoadPreviewSkybox(const uint16_t id,const bool setAsSelected) {

		namespace fs = std::filesystem;

		if (m_entryNames.empty()) {
			MNEMOSY_ERROR("Faild to load preview skybox. List is empty");
			MNEMOSY_POPUP("Faild to load preview skybox. List is empty");
			return new graphics::Skybox();
		}

		if (id >= m_entryNames.size()) {
			MNEMOSY_ERROR("Faild to load preview skybox, Id does not exist");
			return new graphics::Skybox();
		}

		if (setAsSelected) {
			m_currentSelected = id;
		}


		fs::path folder = MnemosyEngine::GetInstance().GetFileDirectories().GetCubemapsPath();

		return systems::LibProcedures::LibEntry_SkyboxMaterial_LoadFromFile(folder, m_entryNames[id],true);
	}
	
	void SkyboxAssetRegistry::AddLibEntryToPreviewSkyboxes(systems::LibEntry* libEntry)
	{
		namespace fs = std::filesystem;

		if (libEntry == nullptr) {
			return;
		}

		// check if entry name already exists in the preview skyboxes
		if (!m_entryNames.empty()) {
			std::string name = libEntry->name;
			for (int i = 0; i < m_entryNames.size(); i++) {

				if (name == m_entryNames[i]) {
					MNEMOSY_ERROR("Cannot add entry '{}' to preview skyboxes because the name already exists.", name);
					MNEMOSY_POPUP("Cannot add entry '{}' to preview skyboxes because the name already exists.", name);
					return;
				}
			}
		}

		// check that both data file and equirectangular exist otherwise we wont be able to store it to preview skyboxes.

		fs::path entryDataFile = systems::LibProcedures::LibEntry_GetDataFilePath(libEntry);
		if (!fs::exists(entryDataFile)) {
			MNEMOSY_ERROR("Cannot add entry to preview skyboxes because the data file is missing. Path: {} ", entryDataFile.generic_string());
			MNEMOSY_POPUP("Cannot add entry to preview skyboxes because the data file is missing.\nPath: {} ", entryDataFile.generic_string());
			return;
		}
				
		fs::path entryFolder = systems::LibProcedures::LibEntry_GetFolderPath(libEntry);

		bool hasTextures = true;
		{
			bool success = false;
			flcrm::JsonSettings entryJsonFile;
			entryJsonFile.FileOpen(success,entryDataFile,jsonKey_header,jsonKey_skybox_description);
			hasTextures = entryJsonFile.ReadBool(success, jsonKey_skybox_textureIsAssigned, false, false);
		}

		std::filesystem::path cubemapsPath = MnemosyEngine::GetInstance().GetFileDirectories().GetCubemapsPath();

		if (hasTextures) {
			
			fs::path equirectangularPath = entryFolder / fs::u8path(libEntry->name + texture_skybox_fileSuffix_equirectangular);
			if (!fs::exists(equirectangularPath)) {
				MNEMOSY_ERROR("Cannot add entry to preview skyboxes because the equirectangular file is missing. Path: {} ", equirectangularPath.generic_string());
				MNEMOSY_POPUP("Cannot add entry to preview skyboxes because the equirectangular file is missing.\nPath: {} ", equirectangularPath.generic_string());
				return;
			}

			fs::path cubeIrradiancePath = entryFolder / fs::u8path(libEntry->name + texture_skybox_fileSuffix_cubeIrradiance);
			fs::path cubePrefilterPath	= entryFolder / fs::u8path(libEntry->name + texture_skybox_fileSuffix_cubePrefilter);
			

			graphics::PictureError picError;

			graphics::PictureInfo picInfo = graphics::Picture::ReadHdr(picError, equirectangularPath.generic_string().c_str(), true, false);
			if (!picError.wasSuccessfull) {
				MNEMOSY_ERROR("Cannot add entry to preview skyboxes. Failed to load hdr equirectangular image.");
				MNEMOSY_POPUP("Cannot add entry to preview skyboxes. Failed to load hdr equirectangular image.");
				return;
			}

			graphics::Texture* equirectangularTex = new graphics::Texture();

			fs::path newEquirectangularPath = cubemapsPath / equirectangularPath.filename();

			// copy equirectangular file - we keep it there in full resolution bc we dont yet have an easy way to downsample images.
			try {
				fs::copy_file(equirectangularPath, newEquirectangularPath);
			}
			catch (fs::filesystem_error err) {
				MNEMOSY_ERROR("System Error, faild to copy file. \nMessage: {}", err.what());
			}

			// copy irradiance file		
			if (fs::exists(cubeIrradiancePath)) {

				fs::path newPath = cubemapsPath / cubeIrradiancePath.filename();
				try {
					fs::copy_file(cubeIrradiancePath, newPath);
				}
				catch (fs::filesystem_error err) {
					MNEMOSY_ERROR("System Error, faild to copy file. \nMessage: {}", err.what());
				}
			}
			else { // generate it new

				graphics::Cubemap cube;
				cube.GenerateOpenGlCubemap_FromEquirecangularTexture(*equirectangularTex, graphics::CubemapType::MNSY_CUBEMAP_TYPE_IRRADIANCE,false,0);

				fs::path newPath = cubemapsPath / cubeIrradiancePath.filename();

				graphics::KtxImage ktx;

				ktx.SaveCubemap(newPath.generic_string().c_str(), cube.GetGlID(), equirectangularTex->GetTextureFormat(), cube.GetResolution(),false);
			}
			
			// copy prefilter file

			if (fs::exists(cubePrefilterPath)) {

				fs::path newPath = cubemapsPath / cubePrefilterPath.filename();
				try {
					fs::copy_file(cubePrefilterPath, newPath);
				}
				catch (fs::filesystem_error err) {
					MNEMOSY_ERROR("System Error, faild to copy file. \nMessage: {}", err.what());
				}
			}
			else {
				graphics::Cubemap cube;
				cube.GenerateOpenGlCubemap_FromEquirecangularTexture(*equirectangularTex, graphics::CubemapType::MNSY_CUBEMAP_TYPE_PREFILTER,false,0);

				fs::path newPath = cubemapsPath / cubePrefilterPath.filename();

				graphics::KtxImage ktx;
				ktx.SaveCubemap(newPath.generic_string().c_str(), cube.GetGlID(), equirectangularTex->GetTextureFormat(), cube.GetResolution(), true);

			}

			delete equirectangularTex;
		}

		// copy data file
		{

			fs::path newPath = cubemapsPath / entryDataFile.filename();
			try {
				fs::copy_file(entryDataFile,newPath);
			}
			catch (fs::filesystem_error err) {
				MNEMOSY_ERROR("System Error, faild to copy file. \nMessage: {}", err.what());
			}
		}

		m_entryNames.push_back(libEntry->name);
	}


	void SkyboxAssetRegistry::RemoveEntry(const uint16_t id) {

		if (m_entryNames.size() == 1) {
			MNEMOSY_POPUP("You can't delete the last skybox preview entry \nbecause one should always persist.");
			MNEMOSY_POPUP("You can't delete the last skybox preview entry \nbecause one should always persist.");
			return;
		}

		if (id >= m_entryNames.size()) {
			MNEMOSY_ERROR("Cannot delete skybox entry with id: {}, because it doesn't exsist", id);
			return;
		}

		if (id == m_currentSelected) {
			m_currentSelected = 0;
		}

		// delete files
		RemoveFilesForEntry(m_entryNames[id]);

		m_entryNames.erase(m_entryNames.begin() + id);

		SaveDataFile();
	}

	// private
	void SkyboxAssetRegistry::LoadDataFile() {

		namespace fs = std::filesystem;

		fs::path dataFilePath = MnemosyEngine::GetInstance().GetFileDirectories().GetDataPath() / fs::path("skyboxQuickselectRegistryData.mnsydata");

		bool success = false;

		flcrm::JsonSettings dataFile;

		dataFile.FileOpen(success,dataFilePath,"Mnemosy Data File","This contains list of skyboxes for the quick selection menu");
		if (!success) {
			MNEMOSY_WARN("Failed to find skyboxQuickselect registry file. Creating new.");
		}


		m_entryNames =  dataFile.ReadVectorString(success,"entries", std::vector<std::string>(),true);

		m_lastSelectedEntry = dataFile.ReadInt(success, "lastSelectedEntry", 0, true);

		m_currentSelected = m_lastSelectedEntry;

		dataFile.FilePrettyPrintSet(true);

		dataFile.FileClose(success, dataFilePath);
	}

	void SkyboxAssetRegistry::SaveDataFile()
	{

		namespace fs = std::filesystem;

		fs::path dataFilePath = MnemosyEngine::GetInstance().GetFileDirectories().GetDataPath() / fs::path("skyboxQuickselectRegistryData.mnsydata");

		bool success = false;

		flcrm::JsonSettings dataFile;

		dataFile.FileOpen(success, dataFilePath, "Mnemosy Data File", "This contains list of skyboxes for the quick selection menu");
		if (!success) {
			MNEMOSY_WARN("Failed to find SkyboxQuickselect registry file. Creating new.");
		}

		dataFile.WriteVectorString(success, "entries",m_entryNames);

		dataFile.WriteInt(success, "lastSelectedEntry", m_currentSelected);


		dataFile.FilePrettyPrintSet(true);

		dataFile.FileClose(success, dataFilePath);
	}

	void SkyboxAssetRegistry::RemoveFilesForEntry(const std::string& name)
	{
		namespace fs = std::filesystem;

		std::filesystem::path folderPath = MnemosyEngine::GetInstance().GetFileDirectories().GetCubemapsPath();

		// data file
		{
			fs::path filePath = folderPath / fs::u8path(name + ".mnsydata");

			if (fs::exists(filePath)) {
				try {
					fs::remove(filePath);
				}
				catch (fs::filesystem_error err) {
					MNEMOSY_WARN("System Error removing file: {},  \nMessage: {}", filePath.generic_string(), err.what());
				}
			}
		}

		// equirectangular map
		{
			fs::path filePath = folderPath / fs::u8path(name + texture_skybox_fileSuffix_equirectangular);

			if (fs::exists(filePath)) {
				try {
					fs::remove(filePath);
				}
				catch (fs::filesystem_error err) {
					MNEMOSY_WARN("System Error removing file: {},  \nMessage: {}", filePath.generic_string(), err.what());
				}
			}
		}

		// irradiance cube map
		{
			fs::path filePath = folderPath / fs::u8path(name + texture_skybox_fileSuffix_cubeIrradiance);

			if (fs::exists(filePath)) {
				try {
					fs::remove(filePath);
				}
				catch (fs::filesystem_error err) {
					MNEMOSY_WARN("System Error removing file: {},  \nMessage: {}", filePath.generic_string(), err.what());
				}
			}
		}
		// prefilter cube map
		{
			fs::path filePath = folderPath / fs::u8path(name + texture_skybox_fileSuffix_cubePrefilter);

			if (fs::exists(filePath)) {
				try {
					fs::remove(filePath);
				}
				catch (fs::filesystem_error err) {
					MNEMOSY_WARN("System Error removing file: {},  \nMessage: {}", filePath.generic_string(), err.what());
				}
			}
		}


	}



} // !mnemosy::systems