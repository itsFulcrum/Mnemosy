#include "Include/Systems/MaterialLibraryRegistry.h"

#include "Include/MnemosyEngine.h"
#include "Include/MnemosyConfig.h"
#include "Include/Core/Log.h"
#include "Include/Core/Clock.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Core/JsonSettings.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/TextureDefinitions.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Utils/KtxImage.h"


#include "Include/Systems/JsonKeys.h"
#include "Include/Systems/ExportManager.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/FolderTree.h"
#include "Include/Systems/TextureGenerationManager.h"
#include "Include/Systems/ThumbnailManager.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/logger.hpp>


#include <glad/glad.h>
#include <fstream>
#include <thread>

namespace mnemosy::systems {
	// == public methods

	MaterialLibraryRegistry::MaterialLibraryRegistry()
		: m_fileDirectories{MnemosyEngine::GetInstance().GetFileDirectories()}
	{
		namespace fs = std::filesystem;

#ifdef MNEMOSY_CONFIG_DEBUG
		prettyPrintMaterialFiles = true;
#endif

		m_folderTree = new FolderTree(jsonLibKey_RootNodeName);

		m_folderNodeOfActiveMaterial = nullptr;


		fs::path pathToUserDirectoriesDataFile = m_fileDirectories.GetUserLibDataFile();

		if (!CheckDataFile(pathToUserDirectoriesDataFile)) {
			MNEMOSY_WARN("Failed to read materialLibraryData file: Creating new empty file at {}", pathToUserDirectoriesDataFile.generic_string());
			SaveUserDirectoriesData();
		}

		float timeStart = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();
		LoadUserDirectoriesFromFile();
		float timeEnd = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();


		unsigned int materialCount = m_folderTree->RecursiveCountMaterials(m_folderTree->GetRootPtr(), 0);

		MNEMOSY_DEBUG("Loaded Material Library in {} Seconds, Loaded {} Material Entries",timeEnd-timeStart, materialCount);


		m_selectedFolderNode = m_folderTree->GetRootPtr();
	}

	MaterialLibraryRegistry::~MaterialLibraryRegistry()
	{

		SaveUserDirectoriesData();

		delete m_folderTree;
		m_folderTree = nullptr;
	}


	void MaterialLibraryRegistry::LoadUserDirectoriesFromFile()
  	{

		std::filesystem::path materialLibraryDataFilePath = m_fileDirectories.GetUserLibDataFile();

		if (!CheckDataFile(materialLibraryDataFilePath)) {

			return;
		}

		std::ifstream dataFileStream;
		dataFileStream.open(materialLibraryDataFilePath);

		nlohmann::json readFile;
		try {
			readFile = nlohmann::json::parse(dataFileStream);
		} catch (nlohmann::json::parse_error err) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadUserDirectoriesFromFile: Error Parsing File. Message: {}", err.what());
			dataFileStream.close();
			return;
		}

		m_folderTree->LoadFromJson(readFile);

		dataFileStream.close();
	}

  	void MaterialLibraryRegistry::SaveUserDirectoriesData()
  	{
		namespace fs = std::filesystem;


		// this function throws warnings if we change the library directory.
		fs::path materialLibraryDataFilePath = m_fileDirectories.GetUserLibDataFile();

		if (CheckDataFile(materialLibraryDataFilePath)) {

			// if data file exists we first make a backup copy
			fs::path copyTo = m_fileDirectories.GetDataPath() / fs::path("MnemosyMaterialLibraryData_LAST_BACKUP.mnsydata");
			try {
				fs::copy(materialLibraryDataFilePath, copyTo, fs::copy_options::overwrite_existing);
			}
			catch(fs::filesystem_error error){
				MNEMOSY_ERROR("MaterialLibraryRegistry::SaverUserDirectoriesData: System error Copying backup file: \nError Message: {}", error.what());
			}

			// then we clear the existing file
			std::ofstream file;
			file.open(materialLibraryDataFilePath);
			file << "";
			file.close();
		}

		nlohmann::json* LibraryDirectoriesJson = m_folderTree->WriteToJson();

		std::ofstream dataFileStream;
		dataFileStream.open(materialLibraryDataFilePath);

		if (prettyPrintDataFile)
			dataFileStream << LibraryDirectoriesJson->dump(4);
		else
			dataFileStream << LibraryDirectoriesJson->dump(-1);

		dataFileStream.close();

		delete LibraryDirectoriesJson;
  	}

	FolderNode* MaterialLibraryRegistry::AddNewFolder(FolderNode* parentNode, std::string& name) {

		FolderNode* node = m_folderTree->CreateNewFolder(parentNode, name);
		// create system folder
		CreateDirectoryForNode(node);

		// we can NOT Call SaveUserDirectoriesData() here because
		// we call this function when initialising and reading from the file..
		return node;
	}

	void MaterialLibraryRegistry::RenameFolder(FolderNode* node, std::string& newName)
	{

		namespace fs = std::filesystem;

		if (node->IsRoot()) {
			MNEMOSY_WARN("You can't change the name of the root directory");
			return;
		}

		// store old path because pathFromRoot is upadeted inside RenameFolder() method
		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();
		fs::path oldPath = libraryDir / fs::path(node->pathFromRoot);
		std::string oldName = node->name;

		// rename internally
		m_folderTree->RenameFolder(node, newName);

		// rename files on disk
		fs::path newPath = libraryDir / fs::path(node->parent->pathFromRoot) / fs::path(node->name);
		try {
			fs::rename(oldPath, newPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::RenameDirectory: System error renaming directory: {} \nError message: {}", oldPath.generic_string(), newPath.generic_string(), error.what());

			// revert name if system can't rename the file on disk
			m_folderTree->RenameFolder(node, oldName);
		}

		// check if the folder  we renamed  included the active material in its hierarchy
		if (!m_activeMaterialDataFilePath.empty()) {

			fs::directory_entry activeMaterialDataFile = fs::directory_entry(m_activeMaterialDataFilePath);
			if (!activeMaterialDataFile.exists()) {
				MNEMOSY_TRACE("MaterialLibraryRegistry::RenameDirectory: active material path doesn exist anymore");
				std::string activeMatName = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial().Name;
				m_activeMaterialDataFilePath = libraryDir / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) /  fs::path(activeMatName) / fs::path(activeMatName + ".mnsydata");
			}
		}
	}

	void MaterialLibraryRegistry::MoveFolder(FolderNode* dragSource, FolderNode* dragTarget) {

		namespace fs = std::filesystem;

		fs::path libraryDir = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();
		fs::path fromPath = libraryDir / fs::path(dragSource->pathFromRoot);
		fs::path toPath = libraryDir / fs::path(dragTarget->pathFromRoot);

		// Copying and then removing works rn but is not very elegant. fs::rename() does not work and throws acces denied error
			//fs::rename(fromPath, toPath / fromPath.filename());

		try { // copy directory to new location
			fs::copy(fromPath, toPath / fromPath.filename(), fs::copy_options::recursive);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveFolder: System Error Copying directory: \nMessage: {}", error.what());
			return;
		}

		try { // remove old directory
			fs::remove_all(fromPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveFolder: System Error Removing old directory: \nMessage: {}", error.what());

			// if this happens we should clean up the copy we just did
			try {
				fs::remove_all(toPath / fromPath.filename());
			}
			catch (fs::filesystem_error error2) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::MoveFolder: System Error Removing directory that was just copied, if this happend something went really wrong! \nMessage: {}", error.what());
			}
			return;
		}

		// Updating Internal Data
		m_folderTree->MoveFolder(dragSource, dragTarget);


		// should prob save to data file here
		SaveUserDirectoriesData();

		// check if the folder  we moved included the active material in its hierarchy by checking if the activeMaterialDataFile still exists at its last location
		if (UserMaterialBound()) {

			fs::directory_entry activeMaterialDataFile;
			try {
				activeMaterialDataFile = fs::directory_entry(m_activeMaterialDataFilePath);
			}
			catch (fs::filesystem_error err) {
				MNEMOSY_TRACE("MaterialLibraryRegistry::MoveDirectory: cant create directory entry {}",err.what());
			}

			if (!activeMaterialDataFile.exists()) {

				//SetDefaultMaterial();
				std::string activeMatName = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial().Name;

				fs::path newLocation;
				try {
					newLocation = libraryDir / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMatName) / fs::path(activeMatName + ".mnsydata");
				}
				catch (fs::filesystem_error err) {
					MNEMOSY_TRACE("MaterialLibraryRegistry::MoveDirectory: cant create path {}", err.what());
				}

				MNEMOSY_TRACE("MaterialLibraryRegistry::MoveDirectory: Active material moved To: {}",newLocation.generic_string());

				m_activeMaterialDataFilePath = newLocation;
			}
		}
	}

	void MaterialLibraryRegistry::DeleteAndKeepChildren(FolderNode* node)
	{
		// never delete root node
		if (node->IsRoot()) {
			MNEMOSY_WARN("MaterialLibraryRegistry::DeleteAndKeepChildren: You can't delete the root directory");
			return;
		}


		//1. move all sub nodes and materials into parent
		if (node->HasMaterials()) {
			std::vector<MaterialInfo*> matsCopy = node->subMaterials;

			for (size_t i = 0; i < matsCopy.size(); i++) {

				MoveMaterial(node, node->parent, matsCopy[i]);
			}
			matsCopy.clear();
		}

		if (!node->IsLeafNode()) {

			std::vector<FolderNode*> subFolderCopy = node->subNodes;
			for (size_t i = 0; i < subFolderCopy.size(); i++) {

				MoveFolder(subFolderCopy[i], node->parent);
			}
			subFolderCopy.clear();
		}

		//2. delete folder
		DeleteFolderHierarchy(node);
	}

	void MaterialLibraryRegistry::DeleteFolderHierarchy(FolderNode* node)
	{

		namespace fs = std::filesystem;

		// Deletes the entire hierarchy of nodes in memory and the files on disk, including the supplied beginning node

		// never delete root node
		if (node->IsRoot()) {
			MNEMOSY_WARN("You can't delete the root directory");
			return;
		}

		{ // delete directories from disk
			fs::path libraryDir = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();
			fs::path directoryPathToDelete = libraryDir / fs::path(node->pathFromRoot);

			try {
				// this call removes all files and directories underneith permanently without moving it to trashbin
				fs::remove_all(directoryPathToDelete);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteFolderHierarchy: System error deleting directory: {} \nError Message: {} ", directoryPathToDelete.generic_string(), error.what());
			}
		}


		OpenFolderNode(node->parent);
		m_folderTree->DeleteFolderHierarchy(node);

		// check if the folder hierarchy we deleted included the active material
		fs::directory_entry activeMaterialDataFile = fs::directory_entry(m_activeMaterialDataFilePath);
		if (!activeMaterialDataFile.exists()) {
			SetDefaultMaterial();
		}
	}

	void MaterialLibraryRegistry::AddNewMaterial(FolderNode* node, std::string& name) {
		namespace fs = std::filesystem;

		MaterialInfo* matInfo = m_folderTree->CreateNewMaterial(node, name);

		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();
		// create directory for material
		fs::path materialDirectory = libraryDir / fs::path(node->pathFromRoot) / fs::path(matInfo->name);

		try {
			fs::create_directory(materialDirectory);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::AddMaterial: System error creating directory. \nError message: {}", error.what());

			int posInVector = -1;
			for (size_t i = 0; i < node->subMaterials.size(); i++) {

				if (node->subMaterials[i]->runtime_ID == matInfo->runtime_ID) {
					posInVector = i;
				}
			}
			MNEMOSY_ASSERT(posInVector != -1, "This should not happen because we just added the material");


			m_folderTree->DeleteMaterial(node, posInVector);

			return;
		}

		// create default material data file

		// Copy Default thumbnail image
		fs::path pathToDefaultThumbnail = m_fileDirectories.GetTexturesPath() / fs::path("default_thumbnail.ktx2");
		fs::path pathToMaterialThumbnail = materialDirectory / fs::path(matInfo->name + texture_fileSuffix_thumbnail);

		try {
			fs::copy_file(pathToDefaultThumbnail, pathToMaterialThumbnail);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::AddMaterial: System error copyingFile. \nError message: {}", error.what());

			return;
		}

		CreateNewMaterialDataFile(materialDirectory, matInfo->name);


		// check if it was created in the currently opend folder
		if (node == m_selectedFolderNode) {
			// make sure it gets loaded

			MnemosyEngine::GetInstance().GetThumbnailManager().AddMaterialForThumbnailing(matInfo);
		}

		SaveUserDirectoriesData();


	}

	void MaterialLibraryRegistry::RenameMaterial(systems::FolderNode* node, systems::MaterialInfo* materialInfo, std::string& newName, int positionInVector) {
		namespace fs = std::filesystem;

		std::string oldName = materialInfo->name;
		unsigned int matID = materialInfo->runtime_ID;

		if (materialInfo->name == newName)
			return;


		// rename material internally
		m_folderTree->RenameMaterial(materialInfo, newName);

		std::string finalName = materialInfo->name;

		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();
		fs::path materialDir = libraryDir / fs::path(node->pathFromRoot) / fs::path(oldName);

		// change name of data file.
		{
			fs::path newDataFilePath = materialDir / fs::path(finalName + ".mnsydata");
			fs::path oldDataFilePath = materialDir / fs::path(oldName + ".mnsydata");

			bool success = false;
			core::JsonSettings matFile;

			matFile.SettingsFileOpen(success,oldDataFilePath,jsonMatKey_header,jsonMatKey_description);
			if(!success)
			{
				MNEMOSY_ERROR("MaterialLibraryRegistry::ChangeMaterialName: Error opening material file {} file. Message: {}",oldDataFilePath.generic_string(), matFile.ErrorStringLastGet());
				return;
			}


			matFile.SettingWriteString(success, jsonMatKey_name,finalName);

			// Renaming all accosiated textures
			// loops through all possilbe texture types as defined in PBRTextureType enum
			for (int i = 0; i < (int)graphics::PBRTextureType::MNSY_TEXTURE_COUNT; i++) {

				std::string assignedKey = graphics::TextureDefinitions::GetJsonMatKey_assigned_FromTextureType((graphics::PBRTextureType)i);

				bool textureTypeIsAssigned = matFile.SettingReadBool(success,assignedKey,false,false);

				if (textureTypeIsAssigned){

					std::string pathJsonKey = graphics::TextureDefinitions::GetJsonMatKey_path_FromTextureType((graphics::PBRTextureType)i);

					std::string oldFileName = matFile.SettingReadString(success,pathJsonKey,jsonMatKey_pathNotAssigned,false);
					std::string newFileName = graphics::TextureDefinitions::GetTextureFileNameFromTextureType(finalName, (graphics::PBRTextureType)i);

					fs::path oldTextureFile = materialDir / fs::path(oldFileName);
					fs::path newTextureFile = materialDir / fs::path(newFileName);

					try {
						fs::rename(oldTextureFile, newTextureFile);
					}
					catch (fs::filesystem_error e) {
						MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());
					}

					matFile.SettingWriteString(success,pathJsonKey,newFileName);
				}
			}

			// now we get all channelpacked textures with this material and rename them too

			bool hasPacked = matFile.SettingReadBool(success, jsonMatKey_hasChannelPacked,false,true);

			if (hasPacked) {

				std::vector<std::string> suffixes = matFile.SettingReadVectorString(success,jsonMatKey_packedSuffixes,std::vector<std::string>(),false);


				if (!suffixes.empty()) {


					// rename channel packed textures
					for (int i = 0; i < suffixes.size(); i++) {

						std::string oldFileName = oldName   + suffixes[i] + texture_textureFileType;
						std::string newFileName = finalName + suffixes[i] + texture_textureFileType;

						fs::path oldFilePath = materialDir / fs::path(oldFileName);
						fs::path newFilePath = materialDir / fs::path(newFileName);

						try {
							fs::rename(oldFilePath, newFilePath);
						}
						catch (fs::filesystem_error e) {
							MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming channelpacked texture file. \nError message: {}", e.what());
						}
					}
				}
			}

			// change name of thumbnail file
			std::string oldFileName = matFile.SettingReadString(success,jsonMatKey_thumbnailPath,jsonMatKey_pathNotAssigned,false);
			std::string newFileName = finalName + texture_fileSuffix_thumbnail;
			fs::path oldTextureFile = materialDir / fs::path(oldFileName);
			fs::path newTextureFile = materialDir / fs::path(newFileName);
			try { fs::rename(oldTextureFile, newTextureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what()); }

			matFile.SettingWriteString(success,jsonMatKey_thumbnailPath,newFileName);

			// outputting updated file

			if(prettyPrintMaterialFiles)
				matFile.SettingsFilePrettyPrintSet(true);

			matFile.SettingsFileClose(success,oldDataFilePath);

			try {fs::rename(oldDataFilePath, newDataFilePath);}
			catch (fs::filesystem_error error) {MNEMOSY_ERROR("MaterialLibraryRegistry::ChangeMaterialName: System error renaming dataFile. \nError message: {}", error.what());}
		}

		// change name of material folder.
		{
			fs::path oldPath = libraryDir / fs::path(node->pathFromRoot) / fs::path(oldName);
			fs::path newPath = libraryDir / fs::path(node->pathFromRoot) / fs::path(finalName);

			try { fs::rename(oldPath, newPath);}
			catch (fs::filesystem_error error) {MNEMOSY_ERROR("MaterialLibraryRegistry::ChangeMaterialName: System error renaming directory. \nError message: {}", error.what());}
		}

		// check if we are changing the active material
		if (m_activeMaterialID == matID) {
			// set updated path for data file
			MnemosyEngine::GetInstance().GetScene().GetActiveMaterial().Name = finalName;
			fs::path newDataPath = libraryDir / fs::path(node->pathFromRoot) / fs::path(finalName) / fs::path(finalName + ".mnsydata");
			m_activeMaterialDataFilePath = newDataPath;
		}

		// save library data file;
		SaveActiveMaterialToFile();
		SaveUserDirectoriesData();

/*
		namespace fs = std::filesystem;

		std::string oldName = materialInfo->name;
		unsigned int matID = materialInfo->runtime_ID;

		if (materialInfo->name == newName)
			return;


		// rename material internally
		m_folderTree->RenameMaterial(materialInfo, newName);

		std::string finalName = materialInfo->name;

		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();
		fs::path materialDir = libraryDir / fs::path(node->pathFromRoot) / fs::path(oldName);

		// change name of data file.
		{
			fs::path newDataFilePath = materialDir / fs::path(finalName + ".mnsydata");
			fs::path oldDataFilePath = materialDir / fs::path(oldName + ".mnsydata");

			std::fstream dataFileStream;
			dataFileStream.open(oldDataFilePath.generic_string());

			nlohmann::json readFile;
			try {
				readFile = nlohmann::json::parse(dataFileStream);
			}
			catch (nlohmann::json::parse_error err) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::ChangeMaterialName: Error parsing json file. Message: {}", err.what());
				return;
			}
			dataFileStream.close();

			readFile[jsonMatKey_name] = finalName;

			// Renaming all accosiated textures
			// loops through possilbe texture types as defined in PBRTextureType enum
			for (size_t i = 0; i <= 7; i++) {

				std::string assignedKey = graphics::TextureDefinitions::GetJsonMatKey_assigned_FromTextureType((graphics::PBRTextureType)i);
				bool textureTypeIsAssigned = readFile[assignedKey].get<bool>();

				if (textureTypeIsAssigned) {


					std::string pathJsonKey = graphics::TextureDefinitions::GetJsonMatKey_path_FromTextureType((graphics::PBRTextureType)i);

					std::string oldFileName = readFile[pathJsonKey].get<std::string>();
					std::string newFileName = graphics::TextureDefinitions::GetTextureFileNameFromTextureType(finalName, (graphics::PBRTextureType)i);

					fs::path oldTextureFile = materialDir / fs::path(oldFileName);
					fs::path newTextureFile = materialDir / fs::path(newFileName);

					try {
						fs::rename(oldTextureFile, newTextureFile);
					}
					catch (fs::filesystem_error e) {
						MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());
					}

					readFile[pathJsonKey] = newFileName;
				}
			}

			// now we get all channelpacked textures with this material and rename them too

			bool hasPacked = readFile[jsonMatKey_hasChannelPacked].get<bool>();

			if (hasPacked) {

				std::vector<std::string> suffixes = readFile[jsonMatKey_packedSuffixes].get<std::vector<std::string>>();


				if (!suffixes.empty()) {



					for (int i = 0; i < suffixes.size(); i++) {

						std::string oldFileName = oldName   + suffixes[i] + texture_textureFileType;
						std::string newFileName = finalName + suffixes[i] + texture_textureFileType;

						fs::path oldFilePath = materialDir / fs::path(oldFileName);
						fs::path newFilePath = materialDir / fs::path(newFileName);

						try {
							fs::rename(oldFilePath, newFilePath);
						}
						catch (fs::filesystem_error e) {
							MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming channelpacked texture file. \nError message: {}", e.what());
						}

					}
				}
			}

			// change name of thumbnail file
			std::string oldFileName = readFile[jsonMatKey_thumbnailPath].get<std::string>();
			std::string newFileName = finalName + texture_fileSuffix_thumbnail;
			fs::path oldTextureFile = materialDir / fs::path(oldFileName);
			fs::path newTextureFile = materialDir / fs::path(newFileName);
			try { fs::rename(oldTextureFile, newTextureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what()); }
			readFile[jsonMatKey_thumbnailPath] = newFileName;



			// outputting updated file
			std::ofstream outFileStream;
			outFileStream.open(oldDataFilePath.generic_string());
			if (prettyPrintMaterialFiles)
				outFileStream << readFile.dump(4);
			else
				outFileStream << readFile.dump(-1);
			outFileStream.close();

			try {fs::rename(oldDataFilePath, newDataFilePath);}
			catch (fs::filesystem_error error) {MNEMOSY_ERROR("MaterialLibraryRegistry::ChangeMaterialName: System error renaming dataFile. \nError message: {}", error.what());}
		}

		// change name of material folder.
		{
			fs::path oldPath = libraryDir / fs::path(node->pathFromRoot) / fs::path(oldName);
			fs::path newPath = libraryDir / fs::path(node->pathFromRoot) / fs::path(finalName);

			try { fs::rename(oldPath, newPath);}
			catch (fs::filesystem_error error) {MNEMOSY_ERROR("MaterialLibraryRegistry::ChangeMaterialName: System error renaming directory. \nError message: {}", error.what());}
		}

		// check if we are changing the active material
		if (m_activeMaterialID == matID) {
			// set updated path for data file
			MnemosyEngine::GetInstance().GetScene().GetActiveMaterial().Name = finalName;
			fs::path newDataPath = libraryDir / fs::path(node->pathFromRoot) / fs::path(finalName) / fs::path(finalName + ".mnsydata");
			m_activeMaterialDataFilePath = newDataPath;
		}

		// save library data file;
		SaveActiveMaterialToFile();
		SaveUserDirectoriesData();
*/
	}

	void MaterialLibraryRegistry::DeleteMaterial(FolderNode* node, systems::MaterialInfo* materialInfo, int positionInVector) {

		namespace fs = std::filesystem;

		unsigned int matID = materialInfo->runtime_ID;

		// check if material is part of the opend folder
		if (node == m_selectedFolderNode) {
			// unload thumbnail

			MnemosyEngine::GetInstance().GetThumbnailManager().RemoveMaterialFromThumbnailing(materialInfo);
		}

		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();

		// delete files
		fs::path pathToMaterialDirectory = libraryDir / fs::path(node->pathFromRoot) / fs::path(materialInfo->name);
		try {
			fs::remove_all(pathToMaterialDirectory);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteMaterial: System error deleting directory \nError message: ", error.what());
			return;
		}

		// deleting from vector
		m_folderTree->DeleteMaterial(node, positionInVector);

		// check if we have deleted the active material
		if (m_activeMaterialID == matID) {
			SetDefaultMaterial();
		}

		// save
		SaveUserDirectoriesData();
	}

	void MaterialLibraryRegistry::MoveMaterial(FolderNode* sourceNode, FolderNode* targetNode, systems::MaterialInfo* materialInfo) {

		namespace fs = std::filesystem;

		// move material folder / copy dir and remove dir
		std::string materialName = materialInfo->name; // temporary storing name here
		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();

		// Copying and then removing works rn but is not very elegant. fs::rename() does not work and throws acces denied error
			//fs::rename(fromPath, toPath / fromPath.filename());

		fs::path fromPath = libraryDir / fs::path(sourceNode->pathFromRoot) / fs::path(materialName);
		fs::path toPath = libraryDir / fs::path(targetNode->pathFromRoot) / fs::path(materialName);
		try { // copy directory to new location
			fs::copy(fromPath, toPath , fs::copy_options::recursive);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveMaterial: System Error Copying directory: \nMessage: {}", error.what());
			return;
		}
		try { // remove old directory
			fs::remove_all(fromPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveMaterial: System Error Removing old directory: \nMessage: {}", error.what());
			return;
		}

		if (sourceNode == m_selectedFolderNode) {

			MnemosyEngine::GetInstance().GetThumbnailManager().RemoveMaterialFromThumbnailing(materialInfo);
		}

		m_folderTree->MoveMaterial(materialInfo, sourceNode, targetNode);

		// check if the moved material was the active material
		if (materialInfo->runtime_ID == m_activeMaterialID) {
			MNEMOSY_TRACE("MaterialLibraryRegistry::MoveMaterial: Moving Material: ID: {}, active ID: {}",materialInfo->runtime_ID,m_activeMaterialID);

			// set updated path for data file
			fs::path newDataFilePath = toPath / fs::path(materialName + ".mnsydata");
			m_activeMaterialDataFilePath = newDataFilePath;
			m_folderNodeOfActiveMaterial = targetNode;
		}

		// make sure thumbnail gets loaded
		if (targetNode == m_selectedFolderNode) {
			MnemosyEngine::GetInstance().GetThumbnailManager().AddMaterialForThumbnailing(targetNode->subMaterials.back());
		}

		// save
		SaveUserDirectoriesData();
	}

	void MaterialLibraryRegistry::GenereateOpacityFromAlbedoAlpha(graphics::Material& activeMat) {

		namespace fs = std::filesystem;

		fs::path opacityMapPath = GetActiveMaterialFolderPath() / fs::path(std::string(activeMat.Name + texture_fileSuffix_opacity));

		//Generate opacity Texture
		MnemosyEngine::GetInstance().GetTextureGenerationManager().GenerateOpacityFromAlbedoAlpha(activeMat, opacityMapPath.generic_string().c_str(), true);

		// load the texture
		graphics::Texture* tex = new graphics::Texture();

		bool success = tex->GenerateFromFile(opacityMapPath.generic_string().c_str(), true, true,graphics::PBRTextureType::MNSY_TEXTURE_OPACITY);

		if (!success) {

			delete tex;
			tex = nullptr;

			// delete texture file we just created
			try {
				fs::remove_all(opacityMapPath);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::GenerateOpacityFromAlbedoAlpha: System error deleting file \nError message: ", error.what());
				return;
			}
		}

		activeMat.assignTexture(graphics::MNSY_TEXTURE_OPACITY, tex);

		SaveActiveMaterialToFile();
	}

	void MaterialLibraryRegistry::GenerateChannelPackedTexture(graphics::Material& activeMat, std::string& suffix, graphics::ChannelPackType packType, graphics::ChannelPackComponent packComponent_R, graphics::ChannelPackComponent packComponent_G, graphics::ChannelPackComponent packComponent_B, graphics::ChannelPackComponent packComponent_A, unsigned int width, unsigned int height) {

		// check if the file extention is valid

		// check against suffixes already taken by mnemosy
		if (
			   suffix == fileSuffix_albedo
			|| suffix == fileSuffix_normal
			|| suffix == fileSuffix_roughness
			|| suffix == fileSuffix_metallic
			|| suffix == fileSuffix_ambientOcclusion
			|| suffix == fileSuffix_emissive
			|| suffix == fileSuffix_height
			|| suffix == fileSuffix_opacity )
		{
			MNEMOSY_WARN("The suffix {} is already taken by another texture. You must choose a different suffix", suffix);
			return;
		}

		// check against suffiexes that already exist in other channel packed textures of the material
		if (activeMat.SuffixExistsInPackedTexturesList(suffix)) {
			MNEMOSY_WARN("The suffix {} is already taken by another texture. You must choose a different suffix", suffix);
			return;
		}

		// generate the texture


		std::string filename = activeMat.Name + suffix + texture_textureFileType; // materialName_suffix.tif
		std::filesystem::path channelPackedExportPath = GetActiveMaterialFolderPath() / std::filesystem::path(filename);

		MnemosyEngine::GetInstance().GetTextureGenerationManager().GenerateChannelPackedTexture(activeMat, channelPackedExportPath.generic_string().c_str(),true,packType,packComponent_R,packComponent_G,packComponent_B,packComponent_A, width,height);

		// Enlist into suffixes of active mat
		activeMat.HasPackedTextures = true;
		activeMat.PackedTexturesSuffixes.push_back(suffix);

		// save to file
		SaveActiveMaterialToFile();

	}

	void MaterialLibraryRegistry::DeleteChannelPackedTexture(graphics::Material& activeMat, std::string suffix) {

		namespace fs = std::filesystem;

		MNEMOSY_ASSERT(activeMat.HasPackedTextures, "This function should not be called if the material does not contain any channel packed textures");
		MNEMOSY_ASSERT(!activeMat.PackedTexturesSuffixes.empty(), "This function should not be called if the material does not contain any channel packed textures, this assert should never happen because we already checked if the texture has packed textures, there must be a bug somewhere when loading or saving the material");

		// delete texture file

		std::string filename = activeMat.Name + suffix + texture_textureFileType;
		fs::path filepath = GetActiveMaterialFolderPath() / fs::path(filename);


		try {
			fs::remove_all(filepath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteChannelPackedTexture: System error deleting file \nError message: ", error.what());
			return;
		}

		// remove from internal list

		// find the index at whicht to remove
		int index = -1;
		for (int i = 0; i < activeMat.PackedTexturesSuffixes.size(); i++) {

			if (suffix == activeMat.PackedTexturesSuffixes[i]) {

				index = i;
				break;
			}
		}

		MNEMOSY_ASSERT(index != -1, "This should not happen, we should always find the correct suffix");
		activeMat.PackedTexturesSuffixes.erase(activeMat.PackedTexturesSuffixes.begin() + index);

		// save the meta data file
		SaveActiveMaterialToFile();

		MNEMOSY_INFO("Deleted channel packed texture {}", filepath.generic_string());

	}

	void MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded(std::filesystem::path& materialDirectory, systems::MaterialInfo* materialInfo, FolderNode* parentNode) {

		namespace fs = std::filesystem;

		double beginTime = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();


		// load json file
		fs::path materialDir = m_fileDirectories.GetLibraryDirectoryPath() / materialDirectory;
		fs::path dataFile = m_fileDirectories.GetLibraryDirectoryPath() / materialDirectory / fs::path(materialInfo->name + ".mnsydata");


		bool success = false;

		core::JsonSettings matFile;		
		matFile.SettingsFileOpen(success, dataFile, jsonMatKey_header, jsonMatKey_description);
		if(!success){
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded: Error opening data file: {} \nMessage: {}", dataFile.generic_string(), matFile.ErrorStringLastGet());
			return;
		}



		graphics::Material* mat = new graphics::Material();

		{
			// First kick off a thread for each assigned texture to load

			// Start a thread for each assigned texture to load it into a openCV matrix which is a member of texture
			// then join thread and load that texture form the matrix and upload to openGl
			// openGl calls must all be from the main thread so we have to do it like this

			// Albedo Map
			bool albedoAssigned = matFile.SettingReadBool(success,jsonMatKey_albedoAssigned,false,false);
			graphics::Texture* albedoTex		= new graphics::Texture();
			std::thread thread_load_albedo;

			if (albedoAssigned) {

				std::string path = materialDir.generic_string() + "/" + matFile.SettingReadString(success,jsonMatKey_albedoPath,jsonMatKey_pathNotAssigned,false);
				thread_load_albedo = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*albedoTex), path);
			}


			// Roughness Map
			bool roughnessAssigned = matFile.SettingReadBool(success,jsonMatKey_roughAssigned,false,false);
			graphics::Texture* roughnessTex		= new graphics::Texture();
			std::thread thread_load_roughness;

			if (roughnessAssigned) {

				std::string path = materialDir.generic_string() + "/" + matFile.SettingReadString(success,jsonMatKey_roughPath,jsonMatKey_pathNotAssigned,false);
				thread_load_roughness = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*roughnessTex),path);
			}

			// Metallic Map
			bool metallicAssigned = matFile.SettingReadBool(success,jsonMatKey_metalAssigned,false,false);
			graphics::Texture* metallicTex		= new graphics::Texture();
			std::thread thread_load_metalllic;


			if (metallicAssigned) {

				std::string path = materialDir.generic_string() + "/" + matFile.SettingReadString(success,jsonMatKey_metalPath,jsonMatKey_pathNotAssigned,false);
				thread_load_metalllic = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*metallicTex), path);
			}

			// Emissive Map
			bool emissiveAssigned = matFile.SettingReadBool(success,jsonMatKey_emissionAssigned,false,false);
			graphics::Texture* emissionTex		= new graphics::Texture();
			std::thread thread_load_emissive;


			if (emissiveAssigned) {

				std::string path = materialDir.generic_string() + "/" + matFile.SettingReadString(success,jsonMatKey_emissionPath,jsonMatKey_pathNotAssigned,false);
				thread_load_emissive = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*emissionTex), path);
			}

			// Normal Map
			bool normalAssigned = matFile.SettingReadBool(success,jsonMatKey_normalAssigned,false,false);
			graphics::Texture* normalTex		= new graphics::Texture();
			std::thread thread_load_normal;

			if (normalAssigned) {

				std::string path = materialDir.generic_string() + "/" + matFile.SettingReadString(success,jsonMatKey_normalPath,jsonMatKey_pathNotAssigned,false);
				thread_load_normal = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*normalTex), path);
			}


			// AO map
			bool aoAssigned = matFile.SettingReadBool(success,jsonMatKey_aoAssigned,false,false);
			graphics::Texture* aoTex			= new graphics::Texture();
			std::thread thread_load_ao;

			if (aoAssigned) {

				std::string path = materialDir.generic_string() + "/" + matFile.SettingReadString(success,jsonMatKey_aoPath,jsonMatKey_pathNotAssigned,false);
				thread_load_ao = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*aoTex), path);
			}


			// Height map
			bool heightAssigned = matFile.SettingReadBool(success,jsonMatKey_heightAssigned,false,false);
			graphics::Texture* heightTex = new graphics::Texture();
			std::thread thread_load_height;

			if (heightAssigned) {

				std::string path = materialDir.generic_string() + "/" + matFile.SettingReadString(success,jsonMatKey_heightPath,jsonMatKey_pathNotAssigned,false);
				thread_load_height = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*heightTex), path);
			}

			// Opacity Map
			bool opacityAssigned = matFile.SettingReadBool(success,jsonMatKey_opacityAssigned,false,false);
			graphics::Texture* opacityTex		= new graphics::Texture();
			std::thread thread_load_opacity;

			if (opacityAssigned) {

				std::string path = materialDir.generic_string() + "/" + matFile.SettingReadString(success,jsonMatKey_opacityPath,jsonMatKey_pathNotAssigned,false);
				thread_load_opacity = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*opacityTex),path);
			}

			// Once all threads are going we let the main thread do the rest of the work

			mat->Name = matFile.SettingReadString(success,jsonMatKey_name,"noName",false);

			mat->HasPackedTextures = matFile.SettingReadBool(success,jsonMatKey_hasChannelPacked,false,true);

			if (mat->HasPackedTextures) {

				mat->PackedTexturesSuffixes = matFile.SettingReadVectorString(success,jsonMatKey_packedSuffixes,std::vector<std::string>(),false); 
			}


			mat->Albedo.r = matFile.SettingReadFloat(success,jsonMatKey_albedo_r,0.8f,true);
			mat->Albedo.g = matFile.SettingReadFloat(success,jsonMatKey_albedo_g,0.8f,true);
			mat->Albedo.b = matFile.SettingReadFloat(success,jsonMatKey_albedo_b,0.8f,true);
			mat->Roughness = matFile.SettingReadFloat(success,jsonMatKey_roughness,0.1f,true);
			mat->IsSmoothnessTexture = matFile.SettingReadBool(success,jsonMatKey_isSmoothness,false,true);

			mat->Metallic = matFile.SettingReadFloat(success,jsonMatKey_metallic,0.0f,true);

			mat->Emission.r = matFile.SettingReadFloat(success,jsonMatKey_emission_r,0.0f,true);
			mat->Emission.g = matFile.SettingReadFloat(success,jsonMatKey_emission_g,0.0f,true);
			mat->Emission.b = matFile.SettingReadFloat(success,jsonMatKey_emission_b,0.0f,true);
			mat->EmissionStrength = matFile.SettingReadFloat(success,jsonMatKey_emissionStrength,0.0f,true);
			mat->UseEmissiveAsMask = matFile.SettingReadBool(success,jsonMatKey_useEmissiveAsMask,false,true);

			mat->NormalStrength = matFile.SettingReadFloat(success,jsonMatKey_normalStrength,1.0f,true);

			int normalFormat = matFile.SettingReadInt(success,jsonMatKey_normalMapFormat,0,true);
			if (normalFormat == 0) {
				mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGL);
			}
			else if (normalFormat == 1) {
				mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_DIRECTX);
			}

			mat->HeightDepth = matFile.SettingReadFloat(success,jsonMatKey_heightDepth,0.0f,true);
			mat->MaxHeight = matFile.SettingReadFloat(success,jsonMatKey_maxHeight,0.0f,true);
			mat->OpacityTreshhold = matFile.SettingReadFloat(success,jsonMatKey_opacityThreshold,0.5f,true);
			mat->UseDitheredAlpha = matFile.SettingReadBool(success,jsonMatKey_useDitheredAlpha,false,true);

			mat->UVTiling.x = matFile.SettingReadFloat(success,jsonMatKey_uvScale_x,1.0f,true);
			mat->UVTiling.y = matFile.SettingReadFloat(success,jsonMatKey_uvScale_y,1.0f,true);


			if (prettyPrintMaterialFiles)
				matFile.SettingsFilePrettyPrintSet(true);

			matFile.SettingsFileClose(success,dataFile);



			// we do this here and not at the top of the file to utilize the time we might be waiting for threads better
			SaveActiveMaterialToFile();

			m_userMaterialBound = true;
			m_activeMaterialDataFilePath = dataFile;
			m_activeMaterialID = materialInfo->runtime_ID;
			m_folderNodeOfActiveMaterial = parentNode;

			// join threads
			if (albedoAssigned) {
				thread_load_albedo.join();
				if (albedoTex->GenerateFromCVMat()) {
					mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_ALBEDO, albedoTex);
				}
				else {
					MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading albedo Texture");
					delete albedoTex;
				}
			}
			else {
				delete albedoTex;
			}

			if (roughnessAssigned) {
				thread_load_roughness.join();
				if (roughnessTex->GenerateFromCVMat()) {
					mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_ROUGHNESS, roughnessTex);
				}
				else {
					MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading roughness Texture");
					delete roughnessTex;
				}
			}
			else {
				delete roughnessTex;
			}

			if (metallicAssigned) {
				thread_load_metalllic.join();
				if (metallicTex->GenerateFromCVMat()) {
					mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_METALLIC, metallicTex);
				}
				else {
					MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading metallic Texture");
					delete metallicTex;
				}
			}
			else {
				delete metallicTex;
			}

			if (emissiveAssigned) {
				thread_load_emissive.join();
				if (emissionTex->GenerateFromCVMat()) {
					mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_EMISSION, emissionTex);
				}
				else {
					MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading emission Texture");
					delete emissionTex;
				}
			}
			else {
				delete emissionTex;
			}

			if (normalAssigned) {
				thread_load_normal.join();
				if (normalTex->GenerateFromCVMat()) {
					mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_NORMAL,normalTex);
				}
				else {
					MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading normal Texture");
					delete normalTex;
				}
			}
			else {
				delete normalTex;
			}

			if (aoAssigned) {
				thread_load_ao.join();
				if (aoTex->GenerateFromCVMat()) {
					mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION, aoTex);
				}
				else {
					MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading ao Texture");
					delete aoTex;
				}
			}
			else {
				delete aoTex;
			}

			if (heightAssigned) {
				thread_load_height.join();
				if (heightTex->GenerateFromCVMat()) {
					mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_HEIGHT, heightTex);
				}
				else {
					MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading height Texture");
					delete heightTex;
				}
			}
			else {
				delete heightTex;
			}

			if (opacityAssigned) {
				thread_load_opacity.join();
				if (opacityTex->GenerateFromCVMat()) {
					mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_OPACITY, opacityTex);
				}
				else {
					MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading opacity Texture");
					delete opacityTex;
				}
			}
			else {
				delete opacityTex;
			}



		}

		MnemosyEngine::GetInstance().GetScene().SetMaterial(mat);

		MNEMOSY_DEBUG("MaterialLibrary: Loaded Material: {}", materialInfo->name);

		double endTime = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();
		MNEMOSY_TRACE("Loaded Material in {} seconds", endTime - beginTime);
	}

	void MaterialLibraryRegistry::SaveActiveMaterialToFile() {
		namespace fs = std::filesystem;

		if (!m_userMaterialBound)
			return;

		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();


		fs::path thumbnailPath = fs::path(activeMat.Name + texture_fileSuffix_thumbnail);

		{ // Render thumbnail of active material
			fs::path libDir = m_fileDirectories.GetLibraryDirectoryPath();
			fs::path thumbnailAbsolutePath = libDir / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name) / thumbnailPath;
			MnemosyEngine::GetInstance().GetThumbnailManager().RenderThumbnailOfActiveMaterial(thumbnailAbsolutePath,m_selectedFolderNode, m_activeMaterialID);
		}

		bool success = false;

		core::JsonSettings matFile;		
		matFile.SettingsFileOpen(success, m_activeMaterialDataFilePath, jsonMatKey_header, jsonMatKey_description);
		if(!success){
			MNEMOSY_WARN("MaterialLibraryRegistry::SaveActiveMaterialToFile: Failed to open material data file. msg: {}", matFile.ErrorStringLastGet());		
		}

		// write all material data to file.

		matFile.SettingWriteBool(success,jsonMatKey_hasChannelPacked, activeMat.HasPackedTextures);

		if (activeMat.HasPackedTextures) {
			matFile.SettingWriteVectorString(success,jsonMatKey_packedSuffixes, activeMat.PackedTexturesSuffixes);
		}

		matFile.SettingErase(success,"1_Mnemosy_Data_File"); // keep this here for now but this key is not longer needed and depricated

		matFile.SettingWriteString(success,jsonMatKey_name, activeMat.Name);
		
		matFile.SettingWriteFloat(success,jsonMatKey_albedo_r, activeMat.Albedo.r);
		matFile.SettingWriteFloat(success,jsonMatKey_albedo_g, activeMat.Albedo.g);
		matFile.SettingWriteFloat(success,jsonMatKey_albedo_b, activeMat.Albedo.b);

		matFile.SettingWriteFloat(success,jsonMatKey_roughness, activeMat.Roughness);
		matFile.SettingWriteBool(success,jsonMatKey_isSmoothness, activeMat.IsSmoothnessTexture);

		matFile.SettingWriteFloat(success,jsonMatKey_metallic, activeMat.Metallic);

		matFile.SettingWriteFloat(success,jsonMatKey_emission_r, activeMat.Emission.r);
		matFile.SettingWriteFloat(success,jsonMatKey_emission_g, activeMat.Emission.g);
		matFile.SettingWriteFloat(success,jsonMatKey_emission_b, activeMat.Emission.b);
		matFile.SettingWriteFloat(success,jsonMatKey_emissionStrength, activeMat.EmissionStrength);
		matFile.SettingWriteBool(success,jsonMatKey_useEmissiveAsMask, activeMat.UseEmissiveAsMask);


		matFile.SettingWriteFloat(success,jsonMatKey_normalStrength, activeMat.NormalStrength);
		matFile.SettingWriteInt(success,jsonMatKey_normalMapFormat, activeMat.GetNormalFormatAsInt()); // 0 = OpenGl, 1 = DirectX

		matFile.SettingWriteFloat(success,jsonMatKey_heightDepth, activeMat.HeightDepth);
		matFile.SettingWriteFloat(success,jsonMatKey_maxHeight, activeMat.MaxHeight);
		matFile.SettingWriteFloat(success,jsonMatKey_opacityThreshold, activeMat.OpacityTreshhold);
		matFile.SettingWriteBool(success,jsonMatKey_useDitheredAlpha, activeMat.UseDitheredAlpha);

		matFile.SettingWriteFloat(success,jsonMatKey_uvScale_x, activeMat.UVTiling.x);
		matFile.SettingWriteFloat(success,jsonMatKey_uvScale_y, activeMat.UVTiling.y);

		// loops through all possilbe texture types as defined in PBRTextureType enum
		// for each type write the texture filename and assigned bool to the file.
		for (int i = 0; i < (int)graphics::PBRTextureType::MNSY_TEXTURE_COUNT; i++) {

			graphics::PBRTextureType currentTextureType = (graphics::PBRTextureType)i;

			bool textureTypeIsAssigned = activeMat.IsTextureTypeAssigned(currentTextureType);

			std::string jsonMatKey_path_ofTextureType = graphics::TextureDefinitions::GetJsonMatKey_path_FromTextureType(currentTextureType);
			std::string jsonMatKey_assigned_ofTextureType = graphics::TextureDefinitions::GetJsonMatKey_assigned_FromTextureType(currentTextureType);

			std::string textureFilePath = jsonMatKey_pathNotAssigned;

			if(textureTypeIsAssigned){
				textureFilePath	=  graphics::TextureDefinitions::GetTextureFileNameFromTextureType(activeMat.Name,currentTextureType);
			}

			matFile.SettingWriteBool(success, jsonMatKey_assigned_ofTextureType, textureTypeIsAssigned);
			matFile.SettingWriteString(success, jsonMatKey_path_ofTextureType, textureFilePath);
		}

		matFile.SettingWriteString(success, jsonMatKey_thumbnailPath,thumbnailPath.generic_string());


		if (prettyPrintMaterialFiles)
			matFile.SettingsFilePrettyPrintSet(true);

		matFile.SettingsFileClose(success, m_activeMaterialDataFilePath);
	}

	void MaterialLibraryRegistry::SetDefaultMaterial() {

		m_userMaterialBound = false;
		m_activeMaterialID = 0;
		m_activeMaterialDataFilePath = std::filesystem::path();
		m_folderNodeOfActiveMaterial = nullptr;


		graphics::Material* defaultMaterial = new graphics::Material();
		MnemosyEngine::GetInstance().GetScene().SetMaterial(defaultMaterial);
	}


	void MaterialLibraryRegistry::LoadTextureForActiveMaterial(graphics::PBRTextureType textureType, std::string& filepath) {

		namespace fs = std::filesystem;

		// check if the filepath extention is a compatible file

		fs::path checkPath = fs::path(filepath);

		fs::directory_entry checkDir = fs::directory_entry(checkPath);

		if (checkDir.exists() && checkDir.is_regular_file()) {


			std::string fileExtention = checkPath.extension().generic_string();


			bool isValid = graphics::TextureDefinitions::IsImageFileExtentionValid(fileExtention);
			if (!isValid) {
				MNEMOSY_WARN("The image filetype {} is not supported.", fileExtention)
					return;
			}

		}
		else {
			MNEMOSY_WARN("filepath is not a file or does not exist. path: {}",filepath);
			return;
		}



		graphics::Texture* tex = new graphics::Texture();
		bool success =  tex->GenerateFromFile(filepath.c_str(),true,true,textureType);

		if (!success) {
			MNEMOSY_ERROR("Loading Texture Failed: {}", filepath);
			delete tex;
			return;
		}

		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();
		systems::ExportManager& exportManager = MnemosyEngine::GetInstance().GetExportManager();


		if (!UserMaterialBound()) {

			activeMat.assignTexture(textureType, tex);
			return;
		}

		fs::path materialDir = m_fileDirectories.GetLibraryDirectoryPath() / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name) ;


		std::string filename;
		fs::path exportPath;
		graphics::TextureFormat format = graphics::MNSY_RGB8;


		if (textureType == graphics::MNSY_TEXTURE_ALBEDO) {

			filename = activeMat.Name + texture_fileSuffix_albedo;
			exportPath = materialDir / fs::path(filename);
			format = graphics::MNSY_RGBA8;
		}
		else if (textureType == graphics::MNSY_TEXTURE_NORMAL) {

			activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGL);
			filename = activeMat.Name + texture_fileSuffix_normal;
			exportPath = materialDir / fs::path(filename);
			format = graphics::MNSY_RGB16;
		}
		else if (textureType == graphics::MNSY_TEXTURE_ROUGHNESS) {

			activeMat.IsSmoothnessTexture = false;
			filename = activeMat.Name + texture_fileSuffix_roughness;
			exportPath = materialDir / fs::path(filename);
			format = graphics::MNSY_R16;
		}
		else if (textureType == graphics::MNSY_TEXTURE_METALLIC) {

			filename = activeMat.Name + texture_fileSuffix_metallic;
			exportPath = materialDir / fs::path(filename);
			format = graphics::MNSY_R16;
		}
		else if (textureType == graphics::MNSY_TEXTURE_AMBIENTOCCLUSION) {

			filename = activeMat.Name + texture_fileSuffix_ambientOcclusion;
			exportPath = materialDir / fs::path(filename);
			format = graphics::MNSY_R16;
		}
		else if (textureType == graphics::MNSY_TEXTURE_EMISSION) {

			filename = activeMat.Name + texture_fileSuffix_emissive;
			exportPath = materialDir / fs::path(filename);
			format = graphics::MNSY_RGB8;
		}
		else if (textureType == graphics::MNSY_TEXTURE_HEIGHT) {

			filename = activeMat.Name + texture_fileSuffix_height;
			exportPath = materialDir / fs::path(filename);
			format = graphics::MNSY_R16;
		}
		else if (textureType == graphics::MNSY_TEXTURE_OPACITY) {

			filename = activeMat.Name + texture_fileSuffix_opacity;
			exportPath = materialDir / fs::path(filename);
			format = graphics::MNSY_R16;
		}

		systems::TextureExportInfo exportInfo = systems::TextureExportInfo(exportPath.generic_string(),tex->GetWidth(), tex->GetHeight(), format);
		exportManager.ExportGlTexture_PngOrTiff(tex->GetID(), exportInfo);


		core::JsonSettings matFile;		

		matFile.SettingsFileOpen(success, m_activeMaterialDataFilePath, jsonMatKey_header, jsonMatKey_description);
		if(!success){
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadTextureForActiveMaterial: Error Opening Data File. Message: {}", matFile.ErrorStringLastGet());
		 	delete tex;
			return;
		}


		// set path to assigned in the data file
		{
			std::string jsonMatKey_path_ofTextureType = graphics::TextureDefinitions::GetJsonMatKey_path_FromTextureType(textureType);
			std::string jsonMatKey_assigned_ofTextureType = graphics::TextureDefinitions::GetJsonMatKey_assigned_FromTextureType(textureType);

			matFile.SettingWriteBool(success,jsonMatKey_assigned_ofTextureType,true);
			matFile.SettingWriteString(success,jsonMatKey_path_ofTextureType,filename);
		}


		if (prettyPrintMaterialFiles)
			matFile.SettingsFilePrettyPrintSet(true);

		matFile.SettingsFileClose(success, m_activeMaterialDataFilePath);

		// load texture to material
		activeMat.assignTexture(textureType, tex);

	}

	void MaterialLibraryRegistry::DeleteTextureOfActiveMaterial(graphics::PBRTextureType textureType)
    {
  		namespace fs = std::filesystem;

		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();

		if (!UserMaterialBound()) { // if its default material // should not be possible using the gui bc default material is not editable 
			activeMat.removeTexture(textureType);
			return;
		}


		bool success = false;

		core::JsonSettings matFile;
		matFile.SettingsFileOpen(success, m_activeMaterialDataFilePath, jsonMatKey_header, jsonMatKey_description);


		fs::path materialDir = m_fileDirectories.GetLibraryDirectoryPath() / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name);



		// find filename of the texture then remove it form disk and reset the path to "notAssigned" in the material meta data file.
		{

			std::string jsonMatKey_path_ofTextureType = graphics::TextureDefinitions::GetJsonMatKey_path_FromTextureType(textureType);


			fs::path textureFilePath = materialDir / fs::path(matFile.SettingReadString(success,jsonMatKey_path_ofTextureType,jsonMatKey_pathNotAssigned,false));
			try { fs::remove(textureFilePath); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			std::string jsonMatKey_assigned_ofTextureType = graphics::TextureDefinitions::GetJsonMatKey_assigned_FromTextureType(textureType);

			matFile.SettingWriteBool(success,jsonMatKey_assigned_ofTextureType,false);
			matFile.SettingWriteString(success,jsonMatKey_path_ofTextureType,jsonMatKey_pathNotAssigned);

			// special case texture types

			if (textureType == graphics::MNSY_TEXTURE_NORMAL) {

				activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGL);
				matFile.SettingWriteInt(success,jsonMatKey_normalMapFormat,0);

			}
			else if(textureType == graphics::MNSY_TEXTURE_ROUGHNESS){
				
				activeMat.IsSmoothnessTexture = false;
				matFile.SettingWriteBool(success,jsonMatKey_isSmoothness,false);
			}
			else if(textureType == graphics::MNSY_TEXTURE_EMISSION){
				activeMat.UseEmissiveAsMask = false;
				matFile.SettingWriteBool(success,jsonMatKey_useEmissiveAsMask,false);
			}			
		}
		
		if (prettyPrintMaterialFiles)
			matFile.SettingsFilePrettyPrintSet(true);

		matFile.SettingsFileClose(success, m_activeMaterialDataFilePath);

		activeMat.removeTexture(textureType);
	}

	void MaterialLibraryRegistry::OpenFolderNode(FolderNode* node){


		SaveActiveMaterialToFile();

		MNEMOSY_ASSERT(node != nullptr, "We cannot set a nullptr node as the selected node");

		m_selectedFolderNode = node;

		systems::ThumbnailManager& thumbnailManager = MnemosyEngine::GetInstance().GetThumbnailManager();

		// unload all currently loaded thumbnails
		thumbnailManager.UnloadAllThumbnails();

		if (m_selectedFolderNode->HasMaterials()) {

			for (int i = 0; i < m_selectedFolderNode->subMaterials.size(); i++) {
				thumbnailManager.AddMaterialForThumbnailing(m_selectedFolderNode->subMaterials[i]);
			}
		}
	}

	void MaterialLibraryRegistry::ClearUserMaterialsAndFolders() {

		// Clearing all User materials and folders from memory but not deleting any files.

		SetDefaultMaterial();
		m_selectedFolderNode = m_folderTree->GetRootPtr();

		m_folderTree->Clear();

		SaveUserDirectoriesData();
	}

	void MaterialLibraryRegistry::ClearInternalTree_OnlyMemory() {

		SetDefaultMaterial();
		m_selectedFolderNode = m_folderTree->GetRootPtr();

		m_folderTree->Clear();
	}

	bool MaterialLibraryRegistry::LoadExistingMnemosyLibrary(std::filesystem::path& pathToDataFile, bool savePermanently, bool deleteCurrentLibrary) {

		namespace fs = std::filesystem;

		// first save current
		SaveUserDirectoriesData();

		// make sure everything is valid
		fs::directory_entry dataFile = fs::directory_entry(pathToDataFile);

		if (pathToDataFile.filename() != fs::path("MnemosyMaterialLibraryData.mnsydata") )
		{
			if ( pathToDataFile.filename() == fs::path("MnemosyMaterialLibraryData_BACKUP_COPY.mnsydata"))
			{
				MNEMOSY_WARN("To use the backup file, delete the other corrupted file and rename this backup to MnemosyMaterialLibraryData.mnsydata - loading this renamed file will create a new backup again.");
			}

			return false;
		}



		if (!dataFile.exists() || !dataFile.is_regular_file()) {
			MNEMOSY_ERROR("The Selected path is not a valid mnemosy data file. Path: {}", pathToDataFile.generic_string());
			return false;
		}

		// data file is most likely valid unless a user has messed with it or its corrupted or it doesnt match the folder library it is stored along


		fs::path libraryFolderPath =  pathToDataFile.parent_path() / fs::path("MnemosyMaterialLibrary");
		fs::directory_entry libraryDirectory = fs::directory_entry(libraryFolderPath);

		if (!libraryDirectory.exists() || !libraryDirectory.is_directory()) {
			MNEMOSY_ERROR("There is no MnemosyMaterialLibrary folder next to the MnemosyMaterialLibraryData.mnsydata data file. Path: {}", pathToDataFile.parent_path().generic_string());
			return false;
		}

		if (fs::is_empty(libraryFolderPath)) {
			MNEMOSY_ERROR("The MnemosyMaterialLibrary folder is empty. Path: {}", libraryFolderPath.generic_string());
			return false;
		}

		// make a backup copy of the data file of the new library


		fs::path copyPath = pathToDataFile.parent_path() / fs::path("MnemosyMaterialLibraryData_BACKUP_COPY.mnsydata");
		fs::copy_file(pathToDataFile, copyPath, fs::copy_options::overwrite_existing);

		// now we first clear the current tree in memory
		ClearInternalTree_OnlyMemory();


		MnemosyEngine::GetInstance().GetFileDirectories().LoadExistingMaterialLibrary(libraryFolderPath,deleteCurrentLibrary,savePermanently);



		// last load the new file data

		LoadUserDirectoriesFromFile();

		return true;
	}


	// ======== Getters ========

	FolderNode* MaterialLibraryRegistry::GetRootFolder() {
		return m_folderTree->GetRootPtr();
	}

	FolderNode* MaterialLibraryRegistry::GetFolderByID(FolderNode* node, const unsigned int id) {

		return m_folderTree->RecursivGetNodeByID(node,id);
	}


	std::filesystem::path MaterialLibraryRegistry::GetActiveMaterialFolderPath() {

		MNEMOSY_ASSERT(UserMaterialBound(), "You should check if active material is bound before calling this Method");

		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();
		return m_fileDirectories.GetLibraryDirectoryPath() / m_folderNodeOfActiveMaterial->pathFromRoot / std::filesystem::path(activeMat.Name);
	}


	std::filesystem::path MaterialLibraryRegistry::GetLibraryPath()
	{
		return m_fileDirectories.GetLibraryDirectoryPath();
	}

	std::filesystem::path MaterialLibraryRegistry::GetFolderPath(FolderNode* node)
	{
		return m_fileDirectories.GetLibraryDirectoryPath() / node->pathFromRoot;
	}

	std::filesystem::path MaterialLibraryRegistry::GetMaterialPath(FolderNode* folderNode, MaterialInfo* matInfo) {
		return m_fileDirectories.GetLibraryDirectoryPath() / folderNode->pathFromRoot / std::filesystem::path(matInfo->name);
	}

	std::vector<std::string> MaterialLibraryRegistry::GetFilepathsOfActiveMat(graphics::Material& activeMat) {

		namespace fs = std::filesystem;

		std::vector<std::string> paths;


		if (!UserMaterialBound()) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::GetFilepathsOfActiveMat: No active material");
			return paths;
		}

		fs::path libDir = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();
		fs::path materialFolder = libDir / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name);


		// include each txture that is assigned to this material
		if (activeMat.isAlbedoAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + texture_fileSuffix_albedo);
			paths.push_back(p.generic_string());
		}
		if (activeMat.isNormalAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + texture_fileSuffix_normal);
			paths.push_back(p.generic_string());
		}
		if (activeMat.isRoughnessAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + texture_fileSuffix_roughness);
			paths.push_back(p.generic_string());
		}
		if (activeMat.isMetallicAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + texture_fileSuffix_metallic);
			paths.push_back(p.generic_string());
		}
		if (activeMat.isAoAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + texture_fileSuffix_ambientOcclusion);
			paths.push_back(p.generic_string());
		}
		if (activeMat.isEmissiveAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + texture_fileSuffix_emissive);
			paths.push_back(p.generic_string());
		}
		if (activeMat.isHeightAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + texture_fileSuffix_height);
			paths.push_back(p.generic_string());
		}
		if (activeMat.isOpacityAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + texture_fileSuffix_opacity);
			paths.push_back(p.generic_string());
		}


		// include channel packed textures as well
		if (activeMat.HasPackedTextures) {

			if (!activeMat.PackedTexturesSuffixes.empty()) {

				for (int i = 0; i < activeMat.PackedTexturesSuffixes.size(); i++) {

					std::string filename = activeMat.Name + activeMat.PackedTexturesSuffixes[i] + texture_textureFileType;
					fs::path p = materialFolder / fs::path(filename);

					paths.push_back(p.generic_string());
				}

			}
		}

		return paths;
	}

	bool MaterialLibraryRegistry::SearchMaterialsForKeyword(const std::string& keyword) {

		return m_folderTree->CollectMaterialsFromSearchKeyword(keyword);
	}

	std::vector<systems::MaterialInfo*>& MaterialLibraryRegistry::GetSearchResultsList() {

		return m_folderTree->GetSearchResultsList();
	}

	// == private methods

	void MaterialLibraryRegistry::CreateNewMaterialDataFile(std::filesystem::path& folderPath, std::string& name) {

		std::filesystem::path materialDataFilePath = folderPath / std::filesystem::path(name + ".mnsydata");

		bool success = false;

		core::JsonSettings matFile;		
		matFile.SettingsFileOpen(success, materialDataFilePath, jsonMatKey_header, jsonMatKey_description);


		matFile.SettingWriteString(success,jsonMatKey_name, name);
		matFile.SettingWriteBool(success,jsonMatKey_hasChannelPacked, false);
		
		matFile.SettingWriteFloat(success,jsonMatKey_albedo_r, 0.8f);
		matFile.SettingWriteFloat(success,jsonMatKey_albedo_g, 0.8f);
		matFile.SettingWriteFloat(success,jsonMatKey_albedo_b, 0.8f);
		
		matFile.SettingWriteFloat(success,jsonMatKey_roughness, 0.1f);
		matFile.SettingWriteFloat(success,jsonMatKey_metallic, 0.0f);

		matFile.SettingWriteFloat(success,jsonMatKey_emission_r, 0.0f);
		matFile.SettingWriteFloat(success,jsonMatKey_emission_g, 0.0f);
		matFile.SettingWriteFloat(success,jsonMatKey_emission_b, 0.0f);
		matFile.SettingWriteFloat(success,jsonMatKey_emissionStrength, 0.0f);
		matFile.SettingWriteBool(success,jsonMatKey_useEmissiveAsMask, false);

		matFile.SettingWriteFloat(success,jsonMatKey_normalStrength, 1.0f);
		matFile.SettingWriteInt(success,jsonMatKey_normalMapFormat, 0);

		matFile.SettingWriteBool(success,jsonMatKey_isSmoothness, false);
		matFile.SettingWriteFloat(success,jsonMatKey_heightDepth, 0.0f);
		matFile.SettingWriteFloat(success,jsonMatKey_maxHeight, 0.0f);
		matFile.SettingWriteFloat(success,jsonMatKey_opacityThreshold, 0.5f);
		matFile.SettingWriteBool(success,jsonMatKey_useDitheredAlpha, false);

		matFile.SettingWriteFloat(success,jsonMatKey_uvScale_x, 1.0f);
		matFile.SettingWriteFloat(success,jsonMatKey_uvScale_y, 1.0f);


		matFile.SettingWriteBool(success,jsonMatKey_albedoAssigned, false);
		matFile.SettingWriteBool(success,jsonMatKey_roughAssigned, false);
		matFile.SettingWriteBool(success,jsonMatKey_metalAssigned, false);
		matFile.SettingWriteBool(success,jsonMatKey_emissionAssigned, false);
		matFile.SettingWriteBool(success,jsonMatKey_normalAssigned, false);
		matFile.SettingWriteBool(success,jsonMatKey_aoAssigned, false);
		matFile.SettingWriteBool(success,jsonMatKey_heightAssigned, false);
		matFile.SettingWriteBool(success,jsonMatKey_opacityAssigned, false);

		matFile.SettingWriteString(success,jsonMatKey_albedoPath, jsonMatKey_pathNotAssigned);
		matFile.SettingWriteString(success,jsonMatKey_roughPath, jsonMatKey_pathNotAssigned);
		matFile.SettingWriteString(success,jsonMatKey_metalPath, jsonMatKey_pathNotAssigned);
		matFile.SettingWriteString(success,jsonMatKey_emissionPath, jsonMatKey_pathNotAssigned);
		matFile.SettingWriteString(success,jsonMatKey_normalPath, jsonMatKey_pathNotAssigned);
		matFile.SettingWriteString(success,jsonMatKey_aoPath, jsonMatKey_pathNotAssigned);
		matFile.SettingWriteString(success,jsonMatKey_heightPath, jsonMatKey_pathNotAssigned);
		matFile.SettingWriteString(success,jsonMatKey_opacityPath, jsonMatKey_pathNotAssigned);

		matFile.SettingWriteString(success,jsonMatKey_thumbnailPath, std::string(name + texture_fileSuffix_thumbnail));


		if (prettyPrintMaterialFiles)
			matFile.SettingsFilePrettyPrintSet(true);

		matFile.SettingsFileClose(success, materialDataFilePath);
	}

	void MaterialLibraryRegistry::CreateDirectoryForNode(FolderNode* node) {

		namespace fs = std::filesystem;

		fs::path libraryDir = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();
		fs::path directoryPath = libraryDir / fs::path(node->pathFromRoot);

		fs::directory_entry newDir;
		try {
			newDir = fs::directory_entry(directoryPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::CreateDirectoryForNode: Error creating initializing Directory {}\n Error Message: {}", directoryPath.generic_string(), error.what());
			return;
		}

		if (!newDir.exists()) {
			fs::create_directories(newDir.path());
		}

	}

	bool MaterialLibraryRegistry::CheckDataFile(const std::filesystem::path& dataFilePath) {

		namespace fs = std::filesystem;

		fs::directory_entry dataFile = fs::directory_entry(dataFilePath);

		if (!dataFile.exists() || !dataFile.is_regular_file()) {

			MNEMOSY_WARN("MaterialLibraryRegistry::CheckDataFile: File did Not Exist or is not a regular file: {} \nCreating new empty file at that location", dataFilePath.generic_string());
			std::ofstream file;
			file.open(dataFilePath);
			file << "";
			file.close();
			return false;
		}

		return true;
	}

} // !mnemosy::systems
