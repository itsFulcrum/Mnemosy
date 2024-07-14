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
#include "Include/Systems/ThumbnailManager.h"

#include <glad/glad.h>
#include <fstream>


namespace mnemosy::systems
{
	// == public methods
 
	MaterialLibraryRegistry::MaterialLibraryRegistry() 
		: m_fileDirectories{MnemosyEngine::GetInstance().GetFileDirectories()}
	{
		m_folderTree = new FolderTree(jsonLibKey_RootNodeName);
		

		fs::path pathToUserDirectoriesDataFile = m_fileDirectories.GetDataPath() / fs::path("UserLibraryDirectories.mnsydata");
		m_userDirectoriesDataFile = fs::directory_entry(pathToUserDirectoriesDataFile); 
		m_folderNodeOfActiveMaterial = nullptr;
	
		double timeStart = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();
		LoadUserDirectoriesFromFile();
		double timeEnd = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();
		MNEMOSY_INFO("Loaded Material Library in {} Seconds",timeEnd-timeStart);

		m_selectedFolderNode = m_folderTree->GetRootPtr();
	}

	MaterialLibraryRegistry::~MaterialLibraryRegistry() {
		
		SaveUserDirectoriesData();

		delete m_folderTree;
		m_folderTree = nullptr;
	}

	
	FolderNode* MaterialLibraryRegistry::AddNewFolder(FolderNode* parentNode, std::string& name) {

		FolderNode* node = m_folderTree->CreateNewFolder(parentNode, name);
		// create system folder 
		CreateDirectoryForNode(node);
		
		// we can NOT Call SaveUserDirectoriesData() here because 
		// we call this function when initialising and reading from the file..
		return node;
	}
	
	void MaterialLibraryRegistry::RenameFolder(FolderNode* node, std::string& newName) {
		
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

	void MaterialLibraryRegistry::DeleteAndKeepChildren(FolderNode* node) {
		// never delete root node
		if (node->IsRoot()) {
			MNEMOSY_WARN("MaterialLibraryRegistry::DeleteAndKeepChildren: You can't delete the root directory");
			return;
		}


		//1. move all sub nodes and materials into parent
		if (node->HasMaterials()) {
			std::vector<MaterialInfo> matsCopy = node->subMaterials;

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

	void MaterialLibraryRegistry::DeleteFolderHierarchy(FolderNode* node) {
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


	FolderNode* MaterialLibraryRegistry::GetRootFolder() {
		return m_folderTree->GetRootPtr();
	}

	FolderNode* MaterialLibraryRegistry::GetFolderByID(FolderNode* node, const unsigned int id) {

		return m_folderTree->RecursivGetNodeByID(node,id);
	}

	void MaterialLibraryRegistry::AddNewMaterial(FolderNode* node, std::string& name) {

		MaterialInfo& matInfo = m_folderTree->CreateNewMaterial(node, name);

		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();
		// create directory for material
		fs::path materialDirectory = libraryDir / fs::path(node->pathFromRoot) / fs::path(matInfo.name);
		//MNEMOSY_TRACE("Add Material: new material directory: {}", materialDirectory.generic_string());

		try {
			fs::create_directory(materialDirectory);
		} 
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::AddMaterial: System error creating directory. \nError message: {}", error.what());

			int posInVector = -1;
			for (size_t i = 0; i < node->subMaterials.size(); i++) {

				if (node->subMaterials[i].runtime_ID == matInfo.runtime_ID) {
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
		fs::path pathToMaterialThumbnail = materialDirectory / fs::path(matInfo.name + texture_fileSuffix_thumbnail);
		
		try {
			fs::copy_file(pathToDefaultThumbnail, pathToMaterialThumbnail);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::AddMaterial: System error copyingFile. \nError message: {}", error.what());

			return;
		}

				



		CreateNewMaterialDataFile(materialDirectory, matInfo.name);

		// check if it was created in the currently opend folder
		if (node == m_selectedFolderNode) {
			// make sure it gets loaded
			MnemosyEngine::GetInstance().GetThumbnailManager().NewThumbnailInActiveFolder();
		}


		SaveUserDirectoriesData();
	}

	void MaterialLibraryRegistry::RenameMaterial(systems::FolderNode* node, systems::MaterialInfo& materialInfo, std::string& newName, int positionInVector) {

		std::string oldName = materialInfo.name;
		unsigned int matID = materialInfo.runtime_ID;

		if (materialInfo.name == newName)
			return;


		// rename material internally
		m_folderTree->RenameMaterial(materialInfo, newName);

		std::string finalName = materialInfo.name;

		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();
		fs::path materialDir = libraryDir / fs::path(node->pathFromRoot) / fs::path(oldName);

		// change name of data file.
		{
			fs::path newDataFilePath = materialDir / fs::path(finalName + ".mnsydata");
			fs::path oldDataFilePath = materialDir / fs::path(oldName + ".mnsydata");
			
			std::fstream dataFileStream;
			dataFileStream.open(oldDataFilePath.generic_string());
			
			json readFile;
			try {
				readFile = json::parse(dataFileStream);
			}
			catch (json::parse_error err) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::ChangeMaterialName: Error parsing json file. Message: {}", err.what());
				return;
			}
			dataFileStream.close();

			readFile[jsonMatKey_name] = finalName;
			
			// Renaming all accosiated textures
			// loops through possilbe texture types as defined in PBRTextureType enum 
			// TODO: extend to 7 to include height and opacity once they are implemented
			for (size_t i = 0; i <= 5; i++) {

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

			// TODO: simplify this into a funciton
			//if (readFile["albedoAssigned"].get<bool>()) {
			//	std::string oldFileName = readFile["albedoPath"].get<std::string>();
			//	std::string newFileName = finalName + "_albedo.tif";
			//	fs::path oldTextureFile = materialDir / fs::path(oldFileName);
			//	fs::path newTextureFile = materialDir / fs::path(newFileName);				
			//	try { fs::rename(oldTextureFile, newTextureFile); } 
			//	catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}				
			//	readFile["albedoPath"] = newFileName;
			//}
			//if (readFile["normalAssigned"].get<bool>()) {
			//	std::string oldFileName = readFile["normalPath"].get<std::string>();
			//	std::string newFileName = finalName + "_normal.tif";
			//	fs::path oldTextureFile = materialDir / fs::path(oldFileName);
			//	fs::path newTextureFile = materialDir / fs::path(newFileName);
			//	try { fs::rename(oldTextureFile, newTextureFile);}
			//	catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}
			//	readFile["normalPath"] = newFileName;
			//}
			//if (readFile["roughAssigned"].get<bool>()) {
			//	std::string oldFileName = readFile["roughPath"].get<std::string>();
			//	std::string newFileName = finalName + "_roughness.tif";
			//	fs::path oldTextureFile = materialDir / fs::path(oldFileName);
			//	fs::path newTextureFile = materialDir / fs::path(newFileName);
			//	try { fs::rename(oldTextureFile, newTextureFile);}
			//	catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}
			//	readFile["roughPath"] = newFileName;
			//}
			//if (readFile["metalAssigned"].get<bool>()) {
			//	std::string oldFileName = readFile["metalPath"].get<std::string>();
			//	std::string newFileName = finalName + "_metallic.tif";
			//	fs::path oldTextureFile = materialDir / fs::path(oldFileName);
			//	fs::path newTextureFile = materialDir / fs::path(newFileName);
			//	try { fs::rename(oldTextureFile, newTextureFile);}
			//	catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what()); }
			//	readFile["metalPath"] = newFileName;
			//}
			//if (readFile["emissionAssigned"].get<bool>()) {
			//	std::string oldFileName = readFile["emissionPath"].get<std::string>();
			//	std::string newFileName = finalName + "_emissive.tif";
			//	fs::path oldTextureFile = materialDir / fs::path(oldFileName);
			//	fs::path newTextureFile = materialDir / fs::path(newFileName);
			//	try { fs::rename(oldTextureFile, newTextureFile);}
			//	catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}
			//	readFile["emissionPath"] = newFileName;
			//}
			//if (readFile["aoAssigned"].get<bool>()) {
			//	std::string oldFileName = readFile["aoPath"].get<std::string>();
			//	std::string newFileName = finalName + "_ambientOcclusion.tif";
			//	fs::path oldTextureFile = materialDir / fs::path(oldFileName);
			//	fs::path newTextureFile = materialDir / fs::path(newFileName);
			//	try {fs::rename(oldTextureFile, newTextureFile);}
			//	catch (fs::filesystem_error e) {MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}
			//	readFile["aoPath"] = newFileName;
			//}



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
			if (prettyPrintDataFile)
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
	}

	void MaterialLibraryRegistry::DeleteMaterial(FolderNode* node, systems::MaterialInfo& materialInfo, int positionInVector) {

		unsigned int matID = materialInfo.runtime_ID;

		// check if material is part of the opend folder
		if (node == m_selectedFolderNode) {
			// unload thumbnail
			MnemosyEngine::GetInstance().GetThumbnailManager().DeleteThumbnailFromCache(materialInfo);
		}

		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();

		// delete files
		fs::path pathToMaterialDirectory = libraryDir / fs::path(node->pathFromRoot) / fs::path(materialInfo.name);
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

	void MaterialLibraryRegistry::MoveMaterial(FolderNode* sourceNode, FolderNode* targetNode, systems::MaterialInfo& materialInfo) {

		// move material folder / copy dir and remove dir
		std::string materialName = materialInfo.name; // temporary storing name here
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
			MnemosyEngine::GetInstance().GetThumbnailManager().DeleteThumbnailFromCache(materialInfo);
		}

		m_folderTree->MoveMaterial(materialInfo, sourceNode, targetNode);

		// check if the moved material was the active material
		if (materialInfo.runtime_ID == m_activeMaterialID) {
			MNEMOSY_TRACE("MaterialLibraryRegistry::MoveMaterial: Moving Material: ID: {}, active ID: {}",materialInfo.runtime_ID,m_activeMaterialID);
			
			// set updated path for data file
			fs::path newDataFilePath = toPath / fs::path(materialName + ".mnsydata");
			m_activeMaterialDataFilePath = newDataFilePath;
			m_folderNodeOfActiveMaterial = targetNode;
		}

		// make sure thumbnail gets loaded
		if (targetNode == m_selectedFolderNode) {
			MnemosyEngine::GetInstance().GetThumbnailManager().NewThumbnailInActiveFolder();
		}

		// save
		SaveUserDirectoriesData();
	}

	void MaterialLibraryRegistry::LoadActiveMaterialFromFile(fs::path& materialDirectory,systems::MaterialInfo& materialInfo,FolderNode* parentNode) {
		
		MNEMOSY_DEBUG("MaterialLibraryRegistry::LoadActiveMaterialFromFile: Loading Material: {}", materialInfo.name);
		// save active material first
		if (m_userMaterialBound) {
			SaveActiveMaterialToFile();
		}
		
		// load json file
		fs::path materialDir = m_fileDirectories.GetLibraryDirectoryPath() / materialDirectory;
		fs::path dataFile = m_fileDirectories.GetLibraryDirectoryPath() / materialDirectory / fs::path(materialInfo.name + ".mnsydata");

		std::ifstream dataFileStream;
		dataFileStream.open(dataFile.generic_string());
		
		json readFile;
		try {
			readFile = json::parse(dataFileStream);
		}
		catch (json::parse_error err) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile: Error parsing data file: {} \nMessage: {}",dataFile.generic_string(), err.what());
			return;
		}

		graphics::Material* mat = new graphics::Material();
		{
			mat->Name		= readFile[jsonMatKey_name].get<std::string>();
			mat->Albedo.r	= readFile[jsonMatKey_albedo_r].get<float>();
			mat->Albedo.g	= readFile[jsonMatKey_albedo_g].get<float>();
			mat->Albedo.b	= readFile[jsonMatKey_albedo_b].get<float>();
			mat->Roughness	= readFile[jsonMatKey_roughness].get<float>();
			mat->Metallic	= readFile[jsonMatKey_metallic].get<float>();

			mat->Emission.r = readFile[jsonMatKey_emission_r].get<float>();
			mat->Emission.g = readFile[jsonMatKey_emission_g].get<float>();
			mat->Emission.b = readFile[jsonMatKey_emission_b].get<float>();
			mat->EmissionStrength = readFile[jsonMatKey_emissionStrength].get<float>();

			mat->NormalStrength = readFile[jsonMatKey_normalStrength].get<float>();

			mat->UVTiling.x = readFile[jsonMatKey_uvScale_x].get<float>();
			mat->UVTiling.y = readFile[jsonMatKey_uvScale_y].get<float>();
		
			bool albedoTexture		= readFile[jsonMatKey_albedoAssigned].get<bool>();
			bool roughnessTexture	= readFile[jsonMatKey_roughAssigned].get<bool>();
			bool metalTexture		= readFile[jsonMatKey_metalAssigned].get<bool>();
			bool emissionTexture	= readFile[jsonMatKey_emissionAssigned].get<bool>();
			bool normalTexture		= readFile[jsonMatKey_normalAssigned].get<bool>();
			bool aoTexture			= readFile[jsonMatKey_aoAssigned].get<bool>();
			//bool heightTexture		= readFile[jsonMatKey_heightAssigned].get<bool>();
			//bool opacityTexture		= readFile[jsonMatKey_opacityAssigned].get<bool>();

			if (albedoTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_albedoPath].get<std::string>();
				graphics::Texture* albedoTex = new graphics::Texture();

				albedoTex->generateFromFile(path.c_str(), true, true);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_ALBEDO, albedoTex);
			}
			if (roughnessTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_roughPath].get<std::string>();
				graphics::Texture* roughnessTex = new graphics::Texture();
				
				roughnessTex->generateFromFile(path.c_str(), true, true);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_ROUGHNESS, roughnessTex);
			}
			if (metalTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_metalPath].get<std::string>();
				graphics::Texture* metallicTex = new graphics::Texture();

				metallicTex->generateFromFile(path.c_str(), true, true);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_METALLIC, metallicTex);
			}
			if (emissionTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_emissionPath].get<std::string>();
				graphics::Texture* emissionTex = new graphics::Texture();
				emissionTex->generateFromFile(path.c_str(), true, true);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_EMISSION, emissionTex);
			}
			if (normalTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_normalPath].get<std::string>();
				graphics::Texture* normalTex = new graphics::Texture();
				normalTex->generateFromFile(path.c_str(), true, true);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_NORMAL, normalTex);

				int normalFormat = readFile[jsonMatKey_normalMapFormat].get<int>();
				if (normalFormat == 0) {
					mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGl);
				}
				else if (normalFormat == 1) {
					mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_DIRECTX);
				}
			}
			if (aoTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_aoPath].get<std::string>();
				graphics::Texture* aoTex = new graphics::Texture();

				aoTex->generateFromFile(path.c_str(), true, true);
				mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION, aoTex);
			}
			// TODO add height and opacity map
			//if (heightTexture) {
			//	std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_heightPath].get<std::string>();
			//	graphics::Texture* heightTex = new graphics::Texture();
			//
			//	heightTex->generateFromFile(path.c_str(), true, true);
			//	mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_HEIGHT, heightTex);
			//}
			//if (opacityTexture) {
			//	std::string path = materialDir.generic_string() + "/" + readFile[jsonMatKey_opacityPath].get<std::string>();
			//	graphics::Texture* opacityTex = new graphics::Texture();
			//
			//	opacityTex->generateFromFile(path.c_str(), true, true);
			//	mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_OPACITY, opacityTex);
			//}


			dataFileStream.close();
		}

		m_userMaterialBound = true;
		m_activeMaterialDataFilePath = dataFile;
		m_activeMaterialID = materialInfo.runtime_ID;
		m_folderNodeOfActiveMaterial = parentNode;

		MnemosyEngine::GetInstance().GetScene().SetMaterial(mat);
	}

	void MaterialLibraryRegistry::SaveActiveMaterialToFile() {

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
		
		json MaterialJson; // top level json object
		MaterialJson[jsonMatKey_MnemosyDataFile] = jsonMatKey_MnemosyDataFileTxt;

		MaterialJson[jsonMatKey_name] = activeMat.Name;
		
		MaterialJson[jsonMatKey_albedo_r] = activeMat.Albedo.r; // maybe convert from range 0-255 to 0-1 range by deviding by 255
		MaterialJson[jsonMatKey_albedo_g] = activeMat.Albedo.g;
		MaterialJson[jsonMatKey_albedo_b] = activeMat.Albedo.b;

		MaterialJson[jsonMatKey_roughness] = activeMat.Roughness;
		MaterialJson[jsonMatKey_metallic] = activeMat.Metallic;

		MaterialJson[jsonMatKey_emission_r] = activeMat.Emission.r;
		MaterialJson[jsonMatKey_emission_g] = activeMat.Emission.r;
		MaterialJson[jsonMatKey_emission_b] = activeMat.Emission.r;
		MaterialJson[jsonMatKey_emissionStrength] = activeMat.EmissionStrength;

		MaterialJson[jsonMatKey_normalStrength] = activeMat.NormalStrength;
		MaterialJson[jsonMatKey_normalMapFormat] = activeMat.GetNormalFormatAsInt(); // 0 = OpenGl, 1 = DirectX
		MaterialJson[jsonMatKey_uvScale_x] = activeMat.UVTiling.x;
		MaterialJson[jsonMatKey_uvScale_y] = activeMat.UVTiling.y;

		bool albedoAssigned		= activeMat.isAlbedoAssigned();
		bool roughAssigned		= activeMat.isRoughnessAssigned();
		bool metalAssigned		= activeMat.isMetallicAssigned();
		bool emissionAssigned	= activeMat.isEmissiveAssigned();
		bool normalAssigned		= activeMat.isNormalAssigned();
		bool aoAssigned			= activeMat.isAoAssigned();
		//bool heightAssinged = activeMat.isHeightAssigned();
		//bool opacityAssigned = activeMat.isOpacityAssigned();

		std::string albedoPath		= jsonMatKey_pathNotAssigned;
		std::string roughnessPath	= jsonMatKey_pathNotAssigned;
		std::string metallicPath	= jsonMatKey_pathNotAssigned;
		std::string emissionPath	= jsonMatKey_pathNotAssigned;
		std::string normalPath		= jsonMatKey_pathNotAssigned;
		std::string aoPath			= jsonMatKey_pathNotAssigned;
		//std::string heightPath	= jsonMatKey_pathNotAssigned;
		//std::string opacityPath	= jsonMatKey_pathNotAssigned;


		std::string matName = activeMat.Name;

		if (albedoAssigned)		{ albedoPath	= matName	+ texture_fileSuffix_albedo; }
		if (roughAssigned)		{ roughnessPath = matName	+ texture_fileSuffix_roughness; }
		if (metalAssigned)		{ metallicPath	= matName	+ texture_fileSuffix_metallic; }
		if (emissionAssigned)	{ emissionPath	= matName	+ texture_fileSuffix_emissive; }
		if (normalAssigned)		{ normalPath	= matName	+ texture_fileSuffix_normal; }
		if (aoAssigned)			{ aoPath		= matName	+ texture_fileSuffix_ambientOcclusion; }
		//if (heightAssigned)		{ heightPath	= matName	+ texture_fileSuffix_height; }
		//if (opacityAssigned)	{ opacityPath	= matName	+ texture_fileSuffix_opacity; }


		MaterialJson[jsonMatKey_albedoAssigned]		= albedoAssigned;
		MaterialJson[jsonMatKey_roughAssigned]		= roughAssigned;
		MaterialJson[jsonMatKey_metalAssigned]		= metalAssigned;
		MaterialJson[jsonMatKey_emissionAssigned]	= emissionAssigned;
		MaterialJson[jsonMatKey_normalAssigned]		= normalAssigned;
		MaterialJson[jsonMatKey_aoAssigned]			= aoAssigned;
		//MaterialJson[jsonMatKey_heightAssigned]	= heightAssigned;
		//MaterialJson[jsonMatKey_opacityAssigned]	= opacityAssigned;

		MaterialJson[jsonMatKey_albedoPath]		= albedoPath;
		MaterialJson[jsonMatKey_roughPath]		= roughnessPath;
		MaterialJson[jsonMatKey_metalPath]		= metallicPath;
		MaterialJson[jsonMatKey_emissionPath]	= emissionPath;
		MaterialJson[jsonMatKey_normalPath]		= normalPath;
		MaterialJson[jsonMatKey_aoPath]			= aoPath;
		//MaterialJson[jsonMatKey_heightPath]		= heightPath;
		//MaterialJson[jsonMatKey_opacityPath]		= opacityPath;



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
		m_activeMaterialDataFilePath = fs::path();
		m_folderNodeOfActiveMaterial = nullptr;


		graphics::Material* defaultMaterial = new graphics::Material();
		MnemosyEngine::GetInstance().GetScene().SetMaterial(defaultMaterial);
	}

	void MaterialLibraryRegistry::LoadTextureForActiveMaterial(graphics::PBRTextureType textureType, std::string& filepath) {
		
		graphics::Texture* tex = new graphics::Texture();
		bool success =  tex->generateFromFile(filepath.c_str(),true,true);		
		
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

		if (textureType == graphics::MNSY_TEXTURE_ALBEDO) {
			
			filename = activeMat.Name + texture_fileSuffix_albedo;
			fs::path exportPath = materialDir / fs::path(filename);
			exportManager.ExportMaterialTexturePngOrTif(exportPath, *tex, false, false);
		}
		else if (textureType == graphics::MNSY_TEXTURE_NORMAL) {
			filename = activeMat.Name + texture_fileSuffix_normal;
			fs::path exportPath = materialDir / fs::path(filename);
			exportManager.ExportMaterialTexturePngOrTif(exportPath, *tex, false, true);
		}
		else if (textureType == graphics::MNSY_TEXTURE_ROUGHNESS) {
			filename = activeMat.Name + texture_fileSuffix_roughness;
			fs::path exportPath = materialDir / fs::path(filename);
			exportManager.ExportMaterialTexturePngOrTif(exportPath, *tex, true, true);
		}
		else if (textureType == graphics::MNSY_TEXTURE_METALLIC) {
			filename = activeMat.Name + texture_fileSuffix_metallic;
			fs::path exportPath = materialDir / fs::path(filename);
			exportManager.ExportMaterialTexturePngOrTif(exportPath, *tex, true, true);
		}
		else if (textureType == graphics::MNSY_TEXTURE_AMBIENTOCCLUSION) {
			filename = activeMat.Name + texture_fileSuffix_ambientOcclusion;
			fs::path exportPath = materialDir / fs::path(filename);
			exportManager.ExportMaterialTexturePngOrTif(exportPath, *tex, true, true);
		}
		else if (textureType == graphics::MNSY_TEXTURE_EMISSION) {
			filename = activeMat.Name + texture_fileSuffix_emissive;
			fs::path exportPath = materialDir / fs::path(filename);
			exportManager.ExportMaterialTexturePngOrTif(exportPath, *tex, false, false);
		}

		// save material data file
		std::ifstream readFileStream;
		readFileStream.open(m_activeMaterialDataFilePath.generic_string());

		json readFile;
		try {
			readFile = json::parse(readFileStream);
		}
		catch (json::parse_error err) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadTextureForActiveMaterial: Error Parsing Data File. Message: {}", err.what());
			readFileStream.close();
			delete tex;
			return;
		}
		readFileStream.close();

		json outFile = readFile;

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


		if (prettyPrintDataFile)
			outFileStream << outFile.dump(4);
		else
			outFileStream << outFile.dump(-1);

		outFileStream.close();
		
		// load texture to material
		activeMat.assignTexture(textureType, tex);
	}

	void MaterialLibraryRegistry::DeleteTextureOfActiveMaterial(graphics::PBRTextureType textureType) {

		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();
		
		if (!UserMaterialBound()) { // if its default aterial
			activeMat.removeTexture(textureType);
			return;
		}
		// read file into json object
		std::ifstream readFileStream;
		readFileStream.open(m_activeMaterialDataFilePath.generic_string());

		json readFile;
		try {
			readFile = json::parse(readFileStream);
		}
		catch (json::parse_error err) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: Error parsing data file.\nError message: {}", err.what());
			readFileStream.close();
			return;
		}
		readFileStream.close();

		// write to json object
		json outFile = readFile;

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

			outFile[jsonMatKey_normalAssigned] = false;
			outFile[jsonMatKey_normalPath] = jsonMatKey_pathNotAssigned;
			outFile[jsonMatKey_normalMapFormat] = 0;
		}
		else if (textureType == graphics::MNSY_TEXTURE_ROUGHNESS) {
			fs::path textureFile = materialDir / fs::path(outFile[jsonMatKey_roughPath].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

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

			activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGl);
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


		std::ofstream outFileStream;
		outFileStream.open(m_activeMaterialDataFilePath.generic_string());

		if (prettyPrintDataFile)
			outFileStream << outFile.dump(4);
		else
			outFileStream << outFile.dump(-1);
		outFileStream.close();
		
		activeMat.removeTexture(textureType);
	}

	fs::path MaterialLibraryRegistry::GetLibraryPath()
	{
		return m_fileDirectories.GetLibraryDirectoryPath();
	}

	fs::path MaterialLibraryRegistry::GetFolderPath(FolderNode* node)
	{
		return m_fileDirectories.GetLibraryDirectoryPath() / node->pathFromRoot;
	}

	fs::path MaterialLibraryRegistry::GetMaterialPath(FolderNode* folderNode, MaterialInfo& matInfo)
	{
		return m_fileDirectories.GetLibraryDirectoryPath() / folderNode->pathFromRoot / fs::path(matInfo.name);
	}

	void MaterialLibraryRegistry::OpenFolderNode(FolderNode* node){
		
		SaveActiveMaterialToFile();

		if (node == nullptr) {
			MNEMOSY_WARN("MaterialLibraryRegistry::OpenFolderNode: Node is nullptr");
			return;
		}

		m_selectedFolderNode = node;

		MnemosyEngine::GetInstance().GetThumbnailManager().DeleteLoadedThumbnailsOfActiveFolder(m_selectedFolderNode);
	}

	void MaterialLibraryRegistry::ClearUserMaterialsAndFolders() {

		// Clearing all User materials and folders from memory but not deleting any files.

		SetDefaultMaterial();
		m_selectedFolderNode = m_folderTree->GetRootPtr();

		m_folderTree->Clear();
		
		SaveUserDirectoriesData();
	}

	std::vector<std::string> MaterialLibraryRegistry::GetFilepathsOfActiveMat(graphics::Material& activeMat)
	{
		std::vector<std::string> paths;


		if (!UserMaterialBound()) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::GetFilepathsOfActiveMat: No active material");
			return paths;
		}

		fs::path libDir = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();
		fs::path materialFolder = libDir / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name);

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

		return paths;

	}

	void MaterialLibraryRegistry::LoadUserDirectoriesFromFile() {

		CheckDataFile(m_userDirectoriesDataFile);

		std::string pathToDataFileString = m_userDirectoriesDataFile.path().generic_string();

		std::ifstream dataFileStream;
		dataFileStream.open(pathToDataFileString);

		json readFile;
		try {
			readFile = json::parse(dataFileStream);
		} catch (json::parse_error err) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadUserDirectoriesFromFile: Error Parsing File. Message: {}", err.what());
			return;
		}
		
		m_folderTree->LoadFromJson(readFile);

		dataFileStream.close();

	}

	void MaterialLibraryRegistry::SaveUserDirectoriesData() {

		std::string pathToDataFileString = m_userDirectoriesDataFile.path().generic_string();
		if (CheckDataFile(m_userDirectoriesDataFile)) {
			// if file exists we clear it first; -> really should think if this is a good idea though
			std::ofstream file;
			file.open(pathToDataFileString);
			file << "";
			file.close();
		}

		std::ofstream dataFileStream;
		dataFileStream.open(pathToDataFileString);

		json* LibraryDirectoriesJson = m_folderTree->WriteToJson();

		if (prettyPrintDataFile)
			dataFileStream << LibraryDirectoriesJson->dump(4);
		else
			dataFileStream << LibraryDirectoriesJson->dump(-1);

		dataFileStream.close();

		delete LibraryDirectoriesJson;
		LibraryDirectoriesJson = nullptr;
	}

	// == private methods

	void MaterialLibraryRegistry::CreateNewMaterialDataFile(fs::path& folderPath, std::string& name) {
		
		fs::path materialDataFile = folderPath / fs::path(name + ".mnsydata");
		
		std::ofstream dataFileStream;
		dataFileStream.open(materialDataFile.generic_string());


		json MaterialJson; // top level json object
		MaterialJson[jsonMatKey_MnemosyDataFile] = jsonMatKey_MnemosyDataFileTxt;
		MaterialJson[jsonMatKey_name]		= name;

		MaterialJson[jsonMatKey_albedo_r]	= 0.8f;
		MaterialJson[jsonMatKey_albedo_g]	= 0.8f;
		MaterialJson[jsonMatKey_albedo_b]	= 0.8f;

		MaterialJson[jsonMatKey_roughness]	= 0.5f;
		MaterialJson[jsonMatKey_metallic]	= 0.0f;

		MaterialJson[jsonMatKey_emission_r]	= 0.0f;
		MaterialJson[jsonMatKey_emission_g]	= 0.0f;
		MaterialJson[jsonMatKey_emission_b]	= 0.0f;
		MaterialJson[jsonMatKey_emissionStrength]	= 0.0f;

		MaterialJson[jsonMatKey_normalStrength]		= 1.0f;
		MaterialJson[jsonMatKey_normalMapFormat]		= 0; // 0 = OpenGl, 1 = DirectX

		MaterialJson[jsonMatKey_uvScale_x]	= 1.0f;
		MaterialJson[jsonMatKey_uvScale_y]	= 1.0f;

		
		MaterialJson[jsonMatKey_albedoAssigned]		= false;
		MaterialJson[jsonMatKey_roughAssigned]		= false;
		MaterialJson[jsonMatKey_metalAssigned]		= false;
		MaterialJson[jsonMatKey_emissionAssigned]	= false;
		MaterialJson[jsonMatKey_normalAssigned]		= false;
		MaterialJson[jsonMatKey_aoAssigned]			= false;
		
		MaterialJson[jsonMatKey_albedoPath]		= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_roughPath]		= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_metalPath]		= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_emissionPath]	= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_normalPath]		= jsonMatKey_pathNotAssigned;
		MaterialJson[jsonMatKey_aoPath]			= jsonMatKey_pathNotAssigned;

		//fs::path thumbnailPath = fs::path(name + jsonMatKey_thumbnailFileName);
		MaterialJson[jsonMatKey_thumbnailPath] = name + texture_fileSuffix_thumbnail;


		if (prettyPrintDataFile)
			dataFileStream << MaterialJson.dump(4);
		else
			dataFileStream << MaterialJson.dump(-1);

		dataFileStream.close();
	}

	void MaterialLibraryRegistry::CreateDirectoryForNode(FolderNode* node) {

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

	bool MaterialLibraryRegistry::CheckDataFile(fs::directory_entry dataFile) {

		std::string pathToDataFileString = dataFile.path().generic_string();

		if (!dataFile.exists())
		{
			MNEMOSY_ERROR("MaterialLibraryRegistry::CheckDataFile: File did Not Exist: {} \nCreating new file at that location", pathToDataFileString);
			std::ofstream file;
			file.open(pathToDataFileString);
			file << "";
			file.close();
			return false;
		}
		if (!dataFile.is_regular_file())
		{
			MNEMOSY_ERROR("MaterialLibraryRegistry::CheckDataFile: File is not a regular file: {} \nCreating new file at that location", pathToDataFileString);
			// maybe need to delete unregular file first idk should never happen anyhow
			std::ofstream file;
			file.open(pathToDataFileString);
			file << "";
			file.close();
			return false;
		}

		return true;
	}

} // !mnemosy::systems