#include "Include/Systems/MaterialLibraryRegistry.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Core/Clock.h"
#include "Include/Core/FileDirectories.h"
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
			return;
		}

		m_folderTree->LoadFromJson(readFile);

		dataFileStream.close();
	}

  	void MaterialLibraryRegistry::SaveUserDirectoriesData()
  	{
		namespace fs = std::filesystem;

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

	void MaterialLibraryRegistry::LoadActiveMaterialFromFile(std::filesystem::path& materialDirectory,systems::MaterialInfo* materialInfo,FolderNode* parentNode)
	{

		namespace fs = std::filesystem;


		MNEMOSY_DEBUG("MaterialLibraryRegistry::LoadActiveMaterialFromFile: Loading Material: {}", materialInfo->name);
		// save active material first
		if (m_userMaterialBound) {
			SaveActiveMaterialToFile();
		}

		// load json file
		fs::path materialDir = m_fileDirectories.GetLibraryDirectoryPath() / materialDirectory;
		fs::path dataFile = m_fileDirectories.GetLibraryDirectoryPath() / materialDirectory / fs::path(materialInfo->name + ".mnsydata");

		std::ifstream dataFileStream;
		dataFileStream.open(dataFile.generic_string());


		nlohmann::json readFile;
		try {
			readFile = nlohmann::json::parse(dataFileStream);
		}
		catch (nlohmann::json::parse_error err) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile: Error parsing data file: {} \nMessage: {}",dataFile.generic_string(), err.what());
			return;
		}

		graphics::Material* mat = new graphics::Material();
		{

			mat->Name		= readFile[jsonMatKey_name].get<std::string>();

			mat->HasPackedTextures = readFile[jsonMatKey_hasChannelPacked].get<bool>();

			if (mat->HasPackedTextures) {

				mat->PackedTexturesSuffixes = readFile[jsonMatKey_packedSuffixes].get<std::vector<std::string>>();
			}

			mat->Albedo.r	= readFile[jsonMatKey_albedo_r].get<float>();
			mat->Albedo.g	= readFile[jsonMatKey_albedo_g].get<float>();
			mat->Albedo.b	= readFile[jsonMatKey_albedo_b].get<float>();
			mat->Roughness	= readFile[jsonMatKey_roughness].get<float>();
			mat->IsSmoothnessTexture = readFile[jsonMatKey_isSmoothness].get<bool>();

			mat->Metallic	= readFile[jsonMatKey_metallic].get<float>();

			mat->Emission.r = readFile[jsonMatKey_emission_r].get<float>();
			mat->Emission.g = readFile[jsonMatKey_emission_g].get<float>();
			mat->Emission.b = readFile[jsonMatKey_emission_b].get<float>();
			mat->EmissionStrength = readFile[jsonMatKey_emissionStrength].get<float>();
			mat->UseEmissiveAsMask = readFile[jsonMatKey_useEmissiveAsMask].get<bool>();

			mat->NormalStrength = readFile[jsonMatKey_normalStrength].get<float>();
			int normalFormat = readFile[jsonMatKey_normalMapFormat].get<int>();
			if (normalFormat == 0) {
				mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGl);
			}
			else if (normalFormat == 1) {
				mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_DIRECTX);
			}

			mat->HeightDepth = readFile[jsonMatKey_heightDepth].get<float>();
			mat->MaxHeight = readFile[jsonMatKey_maxHeight].get<float>();
			mat->OpacityTreshhold = readFile[jsonMatKey_opacityThreshold].get<float>();
			mat->UseDitheredAlpha = readFile[jsonMatKey_useDitheredAlpha].get<bool>();

			mat->UVTiling.x = readFile[jsonMatKey_uvScale_x].get<float>();
			mat->UVTiling.y = readFile[jsonMatKey_uvScale_y].get<float>();

			bool albedoTexture		= readFile[jsonMatKey_albedoAssigned].get<bool>();
			bool roughnessTexture	= readFile[jsonMatKey_roughAssigned].get<bool>();
			bool metalTexture		= readFile[jsonMatKey_metalAssigned].get<bool>();
			bool emissionTexture	= readFile[jsonMatKey_emissionAssigned].get<bool>();
			bool normalTexture		= readFile[jsonMatKey_normalAssigned].get<bool>();
			bool aoTexture			= readFile[jsonMatKey_aoAssigned].get<bool>();
			bool heightTexture		= readFile[jsonMatKey_heightAssigned].get<bool>();
			bool opacityTexture		= readFile[jsonMatKey_opacityAssigned].get<bool>();

			if (albedoTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_albedoPath].get<std::string>();
				graphics::Texture* albedoTex = new graphics::Texture();

				albedoTex->GenerateFromFile(path.c_str(), true, true, graphics::PBRTextureType::MNSY_TEXTURE_ALBEDO);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_ALBEDO, albedoTex);
			}
			if (roughnessTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_roughPath].get<std::string>();
				graphics::Texture* roughnessTex = new graphics::Texture();

				roughnessTex->GenerateFromFile(path.c_str(), true, true, graphics::PBRTextureType::MNSY_TEXTURE_ROUGHNESS);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_ROUGHNESS, roughnessTex);
			}
			if (metalTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_metalPath].get<std::string>();
				graphics::Texture* metallicTex = new graphics::Texture();

				metallicTex->GenerateFromFile(path.c_str(), true, true, graphics::PBRTextureType::MNSY_TEXTURE_METALLIC);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_METALLIC, metallicTex);
			}
			if (emissionTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_emissionPath].get<std::string>();
				graphics::Texture* emissionTex = new graphics::Texture();
				emissionTex->GenerateFromFile(path.c_str(), true, true, graphics::PBRTextureType::MNSY_TEXTURE_EMISSION);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_EMISSION, emissionTex);
			}
			if (normalTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_normalPath].get<std::string>();
				graphics::Texture* normalTex = new graphics::Texture();
				normalTex->GenerateFromFile(path.c_str(), true, true, graphics::PBRTextureType::MNSY_TEXTURE_NORMAL);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_NORMAL, normalTex);
			}
			if (aoTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_aoPath].get<std::string>();
				graphics::Texture* aoTex = new graphics::Texture();

				aoTex->GenerateFromFile(path.c_str(), true, true, graphics::PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION, aoTex);
			}
			if (heightTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_heightPath].get<std::string>();
				graphics::Texture* heightTex = new graphics::Texture();

				heightTex->GenerateFromFile(path.c_str(), true, true, graphics::PBRTextureType::MNSY_TEXTURE_HEIGHT);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_HEIGHT, heightTex);
			}
			if (opacityTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_opacityPath].get<std::string>();
				graphics::Texture* opacityTex = new graphics::Texture();

				opacityTex->GenerateFromFile(path.c_str(), true, true, graphics::PBRTextureType::MNSY_TEXTURE_OPACITY);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_OPACITY, opacityTex);
			}


			dataFileStream.close();
		}

		m_userMaterialBound = true;
		m_activeMaterialDataFilePath = dataFile;
		m_activeMaterialID = materialInfo->runtime_ID;
		m_folderNodeOfActiveMaterial = parentNode;

		MnemosyEngine::GetInstance().GetScene().SetMaterial(mat);
	}

	void MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded(std::filesystem::path& materialDirectory, systems::MaterialInfo* materialInfo, FolderNode* parentNode) {

		namespace fs = std::filesystem;

		double beginTime = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();


		// load json file
		fs::path materialDir = m_fileDirectories.GetLibraryDirectoryPath() / materialDirectory;
		fs::path dataFile = m_fileDirectories.GetLibraryDirectoryPath() / materialDirectory / fs::path(materialInfo->name + ".mnsydata");

		std::ifstream dataFileStream;
		dataFileStream.open(dataFile.generic_string());

		nlohmann::json readFile;
		try {
			readFile = nlohmann::json::parse(dataFileStream);
		}
		catch (nlohmann::json::parse_error err) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded: Error parsing data file: {} \nMessage: {}", dataFile.generic_string(), err.what());
			dataFileStream.close();
			return;
		}
		dataFileStream.close();



		graphics::Material* mat = new graphics::Material();

		{
			// First kick off a thread for each assigned texture to load

			// Start a thread for each assigned texture to load it into a openCV matrix which is a member of texture
			// the join thread and load that texture form the matrix and opload to openGl
			// open gl calls must all be from the main thread so we have to do it like this



			// Albedo Map
			bool albedoAssigned = readFile[jsonMatKey_albedoAssigned].get<bool>();
			graphics::Texture* albedoTex		= new graphics::Texture();
			std::thread thread_load_albedo;

			if (albedoAssigned) {

				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_albedoPath].get<std::string>();
				thread_load_albedo = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*albedoTex), path);
			}


			// Roughness Map
			bool roughnessAssigned = readFile[jsonMatKey_roughAssigned].get<bool>();
			graphics::Texture* roughnessTex		= new graphics::Texture();
			std::thread thread_load_roughness;

			if (roughnessAssigned) {

				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_roughPath].get<std::string>();
				thread_load_roughness = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*roughnessTex),path);
			}

			// Metallic Map
			bool metallicAssigned = readFile[jsonMatKey_metalAssigned].get<bool>();
			graphics::Texture* metallicTex		= new graphics::Texture();
			std::thread thread_load_metalllic;


			if (metallicAssigned) {

				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_metalPath].get<std::string>();
				thread_load_metalllic = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*metallicTex), path);
			}

			// Emissive Map
			bool emissiveAssigned = readFile[jsonMatKey_emissionAssigned].get<bool>();
			graphics::Texture* emissionTex		= new graphics::Texture();
			std::thread thread_load_emissive;


			if (emissiveAssigned) {

				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_emissionPath].get<std::string>();
				thread_load_emissive = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*emissionTex), path);
			}

			// Normal Map
			bool normalAssigned = readFile[jsonMatKey_normalAssigned].get<bool>();
			graphics::Texture* normalTex		= new graphics::Texture();
			std::thread thread_load_normal;

			if (normalAssigned) {

				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_normalPath].get<std::string>();
				thread_load_normal = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*normalTex), path);
			}


			// AO map
			bool aoAssigned = readFile[jsonMatKey_aoAssigned].get<bool>();
			graphics::Texture* aoTex			= new graphics::Texture();
			std::thread thread_load_ao;

			if (aoAssigned) {

				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_aoPath].get<std::string>();
				thread_load_ao = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*aoTex), path);
			}


			// Height map
			bool heightAssigned = readFile[jsonMatKey_heightAssigned].get<bool>();
			graphics::Texture* heightTex = new graphics::Texture();
			std::thread thread_load_height;

			if (heightAssigned) {

				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_heightPath].get<std::string>();
				thread_load_height = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*heightTex), path);
			}

			// Opacity Map
			bool opacityAssigned = readFile[jsonMatKey_opacityAssigned].get<bool>();
			graphics::Texture* opacityTex		= new graphics::Texture();
			std::thread thread_load_opacity;

			if (opacityAssigned) {

				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_opacityPath].get<std::string>();
				thread_load_opacity = std::thread(&graphics::Texture::LoadIntoCVMat, std::ref(*opacityTex),path);
			}

			// Once all threads are going we let the main thread do the rest of the work

			mat->Name = readFile[jsonMatKey_name].get<std::string>();

			mat->HasPackedTextures = readFile[jsonMatKey_hasChannelPacked].get<bool>();

			if (mat->HasPackedTextures) {

				mat->PackedTexturesSuffixes = readFile[jsonMatKey_packedSuffixes].get<std::vector<std::string>>();
			}


			mat->Albedo.r = readFile[jsonMatKey_albedo_r].get<float>();
			mat->Albedo.g = readFile[jsonMatKey_albedo_g].get<float>();
			mat->Albedo.b = readFile[jsonMatKey_albedo_b].get<float>();
			mat->Roughness = readFile[jsonMatKey_roughness].get<float>();
			mat->IsSmoothnessTexture = readFile[jsonMatKey_isSmoothness].get<bool>();

			mat->Metallic = readFile[jsonMatKey_metallic].get<float>();

			mat->Emission.r = readFile[jsonMatKey_emission_r].get<float>();
			mat->Emission.g = readFile[jsonMatKey_emission_g].get<float>();
			mat->Emission.b = readFile[jsonMatKey_emission_b].get<float>();
			mat->EmissionStrength = readFile[jsonMatKey_emissionStrength].get<float>();
			mat->UseEmissiveAsMask = readFile[jsonMatKey_useEmissiveAsMask].get<bool>();

			mat->NormalStrength = readFile[jsonMatKey_normalStrength].get<float>();
			int normalFormat = readFile[jsonMatKey_normalMapFormat].get<int>();
			if (normalFormat == 0) {
				mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGl);
			}
			else if (normalFormat == 1) {
				mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_DIRECTX);
			}

			mat->HeightDepth = readFile[jsonMatKey_heightDepth].get<float>();
			mat->MaxHeight = readFile[jsonMatKey_maxHeight].get<float>();
			mat->OpacityTreshhold = readFile[jsonMatKey_opacityThreshold].get<float>();
			mat->UseDitheredAlpha = readFile[jsonMatKey_useDitheredAlpha].get<bool>();

			mat->UVTiling.x = readFile[jsonMatKey_uvScale_x].get<float>();
			mat->UVTiling.y = readFile[jsonMatKey_uvScale_y].get<float>();

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

		std::ofstream dataFileStream;
		dataFileStream.open(m_activeMaterialDataFilePath.generic_string());

		nlohmann::json MaterialJson; // top level json object
		MaterialJson[jsonMatKey_MnemosyDataFile] = jsonMatKey_MnemosyDataFileTxt;

		MaterialJson[jsonMatKey_hasChannelPacked] = activeMat.HasPackedTextures;

		if (activeMat.HasPackedTextures) {
			MaterialJson[jsonMatKey_packedSuffixes] = activeMat.PackedTexturesSuffixes;
		}


		MaterialJson[jsonMatKey_name] = activeMat.Name;

		MaterialJson[jsonMatKey_albedo_r] = activeMat.Albedo.r; // maybe convert from range 0-255 to 0-1 range by deviding by 255
		MaterialJson[jsonMatKey_albedo_g] = activeMat.Albedo.g;
		MaterialJson[jsonMatKey_albedo_b] = activeMat.Albedo.b;

		MaterialJson[jsonMatKey_roughness] = activeMat.Roughness;
		MaterialJson[jsonMatKey_isSmoothness] = activeMat.IsSmoothnessTexture;

		MaterialJson[jsonMatKey_metallic] = activeMat.Metallic;

		MaterialJson[jsonMatKey_emission_r] = activeMat.Emission.r;
		MaterialJson[jsonMatKey_emission_g] = activeMat.Emission.g;
		MaterialJson[jsonMatKey_emission_b] = activeMat.Emission.b;
		MaterialJson[jsonMatKey_emissionStrength] = activeMat.EmissionStrength;
		MaterialJson[jsonMatKey_useEmissiveAsMask] = activeMat.UseEmissiveAsMask;


		MaterialJson[jsonMatKey_normalStrength] = activeMat.NormalStrength;
		MaterialJson[jsonMatKey_normalMapFormat] = activeMat.GetNormalFormatAsInt(); // 0 = OpenGl, 1 = DirectX

		MaterialJson[jsonMatKey_heightDepth] = activeMat.HeightDepth;
		MaterialJson[jsonMatKey_maxHeight] = activeMat.MaxHeight;
		MaterialJson[jsonMatKey_opacityThreshold] = activeMat.OpacityTreshhold;
		MaterialJson[jsonMatKey_useDitheredAlpha] = activeMat.UseDitheredAlpha;
		
		MaterialJson[jsonMatKey_uvScale_x] = activeMat.UVTiling.x;
		MaterialJson[jsonMatKey_uvScale_y] = activeMat.UVTiling.y;

		bool albedoAssigned		= activeMat.isAlbedoAssigned();
		bool roughAssigned		= activeMat.isRoughnessAssigned();
		bool metalAssigned		= activeMat.isMetallicAssigned();
		bool emissionAssigned	= activeMat.isEmissiveAssigned();
		bool normalAssigned		= activeMat.isNormalAssigned();
		bool aoAssigned			= activeMat.isAoAssigned();
		bool heightAssigned		= activeMat.isHeightAssigned();
		bool opacityAssigned	= activeMat.isOpacityAssigned();

		std::string albedoPath		= jsonMatKey_pathNotAssigned;
		std::string roughnessPath	= jsonMatKey_pathNotAssigned;
		std::string metallicPath	= jsonMatKey_pathNotAssigned;
		std::string emissionPath	= jsonMatKey_pathNotAssigned;
		std::string normalPath		= jsonMatKey_pathNotAssigned;
		std::string aoPath			= jsonMatKey_pathNotAssigned;
		std::string heightPath		= jsonMatKey_pathNotAssigned;
		std::string opacityPath		= jsonMatKey_pathNotAssigned;


		std::string matName = activeMat.Name;

		if (albedoAssigned)		{ albedoPath	= matName	+ texture_fileSuffix_albedo; }
		if (roughAssigned)		{ roughnessPath = matName	+ texture_fileSuffix_roughness; }
		if (metalAssigned)		{ metallicPath	= matName	+ texture_fileSuffix_metallic; }
		if (emissionAssigned)	{ emissionPath	= matName	+ texture_fileSuffix_emissive; }
		if (normalAssigned)		{ normalPath	= matName	+ texture_fileSuffix_normal; }
		if (aoAssigned)			{ aoPath		= matName	+ texture_fileSuffix_ambientOcclusion; }
		if (heightAssigned)		{ heightPath	= matName	+ texture_fileSuffix_height; }
		if (opacityAssigned)	{ opacityPath	= matName	+ texture_fileSuffix_opacity; }


		MaterialJson[jsonMatKey_albedoAssigned]		= albedoAssigned;
		MaterialJson[jsonMatKey_roughAssigned]		= roughAssigned;
		MaterialJson[jsonMatKey_metalAssigned]		= metalAssigned;
		MaterialJson[jsonMatKey_emissionAssigned]	= emissionAssigned;
		MaterialJson[jsonMatKey_normalAssigned]		= normalAssigned;
		MaterialJson[jsonMatKey_aoAssigned]			= aoAssigned;
		MaterialJson[jsonMatKey_heightAssigned]		= heightAssigned;
		MaterialJson[jsonMatKey_opacityAssigned]	= opacityAssigned;

		MaterialJson[jsonMatKey_albedoPath]		= albedoPath;
		MaterialJson[jsonMatKey_roughPath]		= roughnessPath;
		MaterialJson[jsonMatKey_metalPath]		= metallicPath;
		MaterialJson[jsonMatKey_emissionPath]	= emissionPath;
		MaterialJson[jsonMatKey_normalPath]		= normalPath;
		MaterialJson[jsonMatKey_aoPath]			= aoPath;
		MaterialJson[jsonMatKey_heightPath]		= heightPath;
		MaterialJson[jsonMatKey_opacityPath]	= opacityPath;



		MaterialJson[jsonMatKey_thumbnailPath] = thumbnailPath.generic_string();

		if (prettyPrintMaterialFiles)
			dataFileStream << MaterialJson.dump(4);
		else
			dataFileStream << MaterialJson.dump(-1);
		dataFileStream.close();

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

			activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGl);
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



		// save material data file
		std::ifstream readFileStream;
		readFileStream.open(m_activeMaterialDataFilePath.generic_string());

		nlohmann::json readFile;
		try {
			readFile = nlohmann::json::parse(readFileStream);
		}
		catch (nlohmann::json::parse_error err) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadTextureForActiveMaterial: Error Parsing Data File. Message: {}", err.what());
			readFileStream.close();
			delete tex;
			return;
		}
		readFileStream.close();

		nlohmann::json outFile = readFile;

		std::ofstream outFileStream;
		outFileStream.open(m_activeMaterialDataFilePath.generic_string());

		if (textureType == graphics::MNSY_TEXTURE_ALBEDO) {
			outFile[jsonMatKey_albedoAssigned] = true;
			outFile[jsonMatKey_albedoPath] = filename;
		}
		else if (textureType == graphics::MNSY_TEXTURE_NORMAL){
			outFile[jsonMatKey_normalAssigned] = true;
			outFile[jsonMatKey_normalPath] = filename;
		}
		else if (textureType == graphics::MNSY_TEXTURE_ROUGHNESS) {
			outFile[jsonMatKey_roughAssigned] = true;
			outFile[jsonMatKey_roughPath] = filename;
		}
		else if (textureType == graphics::MNSY_TEXTURE_METALLIC) {
			outFile[jsonMatKey_metalAssigned] = true;
			outFile[jsonMatKey_metalPath] = filename;
		}
		else if (textureType == graphics::MNSY_TEXTURE_AMBIENTOCCLUSION) {
			outFile[jsonMatKey_aoAssigned] = true;
			outFile[jsonMatKey_aoPath] = filename;
		}
		else if (textureType == graphics::MNSY_TEXTURE_EMISSION) {
			outFile[jsonMatKey_emissionAssigned] = true;
			outFile[jsonMatKey_emissionPath] = filename;
		}
		else if (textureType == graphics::MNSY_TEXTURE_HEIGHT) {
			outFile[jsonMatKey_heightAssigned] = true;
			outFile[jsonMatKey_heightPath] = filename;
		}
		else if (textureType == graphics::MNSY_TEXTURE_OPACITY) {
			outFile[jsonMatKey_opacityAssigned] = true;
			outFile[jsonMatKey_opacityPath] = filename;
		}


		if (prettyPrintMaterialFiles)
			outFileStream << outFile.dump(4);
		else
			outFileStream << outFile.dump(-1);

		outFileStream.close();

		// load texture to material
		activeMat.assignTexture(textureType, tex);

	}

	void MaterialLibraryRegistry::DeleteTextureOfActiveMaterial(graphics::PBRTextureType textureType)
  {


		namespace fs = std::filesystem;

		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();

		if (!UserMaterialBound()) { // if its default aterial
			activeMat.removeTexture(textureType);
			return;
		}
		// read file into json object
		std::ifstream readFileStream;
		readFileStream.open(m_activeMaterialDataFilePath.generic_string());

		nlohmann::json readFile;
		try {
			readFile = nlohmann::json::parse(readFileStream);
		}
		catch (nlohmann::json::parse_error err) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: Error parsing data file.\nError message: {}", err.what());
			readFileStream.close();
			return;
		}
		readFileStream.close();

		// write to json object
		nlohmann::json outFile = readFile;

		fs::path materialDir = m_fileDirectories.GetLibraryDirectoryPath() / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name);

		if (textureType == graphics::MNSY_TEXTURE_ALBEDO) {
			fs::path textureFile = materialDir / fs::path(outFile[jsonMatKey_albedoPath].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			outFile[jsonMatKey_albedoAssigned] = false;
			outFile[jsonMatKey_albedoPath] = jsonMatKey_pathNotAssigned;
		}
		else if (textureType == graphics::MNSY_TEXTURE_NORMAL) {
			fs::path textureFile = materialDir / fs::path(outFile[jsonMatKey_normalPath].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGl);

			outFile[jsonMatKey_normalAssigned] = false;
			outFile[jsonMatKey_normalPath] = jsonMatKey_pathNotAssigned;
			outFile[jsonMatKey_normalMapFormat] = 0;
		}
		else if (textureType == graphics::MNSY_TEXTURE_ROUGHNESS) {
			fs::path textureFile = materialDir / fs::path(outFile[jsonMatKey_roughPath].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			activeMat.IsSmoothnessTexture = false;

			outFile[jsonMatKey_isSmoothness] = false;
			outFile[jsonMatKey_roughAssigned] = false;
			outFile[jsonMatKey_roughPath] = jsonMatKey_pathNotAssigned;
		}
		else if (textureType == graphics::MNSY_TEXTURE_METALLIC) {
			fs::path textureFile = materialDir / fs::path(outFile[jsonMatKey_metalPath].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			outFile[jsonMatKey_metalAssigned] = false;
			outFile[jsonMatKey_metalPath] = jsonMatKey_pathNotAssigned;
		}
		else if (textureType == graphics::MNSY_TEXTURE_AMBIENTOCCLUSION) {
			fs::path textureFile = materialDir / fs::path(outFile[jsonMatKey_aoPath].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			outFile[jsonMatKey_aoAssigned] = false;
			outFile[jsonMatKey_aoPath] = jsonMatKey_pathNotAssigned;
		}
		else if (textureType == graphics::MNSY_TEXTURE_EMISSION) {
			fs::path textureFile = materialDir / fs::path(outFile[jsonMatKey_emissionPath].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			outFile[jsonMatKey_emissionAssigned] = false;
			outFile[jsonMatKey_emissionPath] = jsonMatKey_pathNotAssigned;
		}
		else if (textureType == graphics::MNSY_TEXTURE_HEIGHT) {
			fs::path textureFile = materialDir / fs::path(outFile[jsonMatKey_heightPath].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			outFile[jsonMatKey_heightAssigned] = false;
			outFile[jsonMatKey_heightPath] = jsonMatKey_pathNotAssigned;
		}
		else if (textureType == graphics::MNSY_TEXTURE_OPACITY) {
			fs::path textureFile = materialDir / fs::path(outFile[jsonMatKey_opacityPath].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			activeMat.UseEmissiveAsMask = false;
			outFile[jsonMatKey_useEmissiveAsMask] = false;
			outFile[jsonMatKey_opacityAssigned] = false;
			outFile[jsonMatKey_opacityPath] = jsonMatKey_pathNotAssigned;
		}



		std::ofstream outFileStream;
		outFileStream.open(m_activeMaterialDataFilePath.generic_string());

		if (prettyPrintMaterialFiles)
			outFileStream << outFile.dump(4);
		else
			outFileStream << outFile.dump(-1);
		outFileStream.close();

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

		std::filesystem::path materialDataFile = folderPath / std::filesystem::path(name + ".mnsydata");

		std::ofstream dataFileStream;
		dataFileStream.open(materialDataFile.generic_string());


		nlohmann::json MaterialJson; // top level json object
		MaterialJson[jsonMatKey_MnemosyDataFile] = jsonMatKey_MnemosyDataFileTxt;
		MaterialJson[jsonMatKey_name]		= name;

		MaterialJson[jsonMatKey_hasChannelPacked] = false;

		MaterialJson[jsonMatKey_albedo_r]	= 0.8f;
		MaterialJson[jsonMatKey_albedo_g]	= 0.8f;
		MaterialJson[jsonMatKey_albedo_b]	= 0.8f;

		MaterialJson[jsonMatKey_roughness]	= 0.1f;
		MaterialJson[jsonMatKey_metallic]	= 0.0f;

		MaterialJson[jsonMatKey_emission_r]	= 0.0f;
		MaterialJson[jsonMatKey_emission_g]	= 0.0f;
		MaterialJson[jsonMatKey_emission_b]	= 0.0f;
		MaterialJson[jsonMatKey_emissionStrength]	= 0.0f;
		MaterialJson[jsonMatKey_useEmissiveAsMask]	= false;


		MaterialJson[jsonMatKey_normalStrength]		= 1.0f;
		MaterialJson[jsonMatKey_normalMapFormat]	= 0; // 0 = OpenGl, 1 = DirectX

		MaterialJson[jsonMatKey_isSmoothness]		= false;

		MaterialJson[jsonMatKey_heightDepth]		= 0.0f;
		MaterialJson[jsonMatKey_maxHeight]			= 0.0f;
		MaterialJson[jsonMatKey_opacityThreshold]	= 0.5;
		MaterialJson[jsonMatKey_useDitheredAlpha]	= false;

		MaterialJson[jsonMatKey_uvScale_x]	= 1.0f;
		MaterialJson[jsonMatKey_uvScale_y]	= 1.0f;

		MaterialJson[jsonMatKey_albedoAssigned]		= false;
		MaterialJson[jsonMatKey_roughAssigned]		= false;
		MaterialJson[jsonMatKey_metalAssigned]		= false;
		MaterialJson[jsonMatKey_emissionAssigned]	= false;
		MaterialJson[jsonMatKey_normalAssigned]		= false;
		MaterialJson[jsonMatKey_aoAssigned]			= false;
		MaterialJson[jsonMatKey_heightAssigned]		= false;
		MaterialJson[jsonMatKey_opacityAssigned]	= false;

		MaterialJson[jsonMatKey_albedoPath]		= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_roughPath]		= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_metalPath]		= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_emissionPath]	= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_normalPath]		= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_aoPath]			= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_heightPath]		= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_opacityPath]	= jsonMatKey_pathNotAssigned;

		MaterialJson[jsonMatKey_thumbnailPath] = name + texture_fileSuffix_thumbnail;


		if (prettyPrintMaterialFiles)
			dataFileStream << MaterialJson.dump(4);
		else
			dataFileStream << MaterialJson.dump(-1);

		dataFileStream.close();
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

			MNEMOSY_ERROR("MaterialLibraryRegistry::CheckDataFile: File did Not Exist or is not a regular file: {} \nCreating new empty file at that location", dataFilePath.generic_string());
			std::ofstream file;
			file.open(dataFilePath);
			file << "";
			file.close();
			return false;
		}

		return true;
	}

} // !mnemosy::systems
