#include "Include/Systems/MaterialLibraryRegistry.h"

#include "Include/Core/Log.h"
#include "Include/Core/Clock.h"
#include "Include/MnemosyEngine.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Utils/KtxImage.h"
#include "Include/Systems/ExportManager.h"

#include "Include/Core/FileDirectories.h"
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
		m_folderTree = new FolderTree("Root");
		

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
		fs::path pathToMaterialThumbnail = materialDirectory / fs::path(matInfo.name + "_thumbnail.ktx2");
		fs::copy_file(pathToDefaultThumbnail,pathToMaterialThumbnail);
						
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

			readFile["name"] = finalName;
			
			// TODO: simplify this into a funciton
			// Renaming all accosiated textures
			if (readFile["albedoAssigned"].get<bool>()) {
				std::string oldFileName = readFile["albedoPath"].get<std::string>();
				std::string newFileName = finalName + "_albedo.tif";
				fs::path oldTextureFile = materialDir / fs::path(oldFileName);
				fs::path newTextureFile = materialDir / fs::path(newFileName);				
				try { fs::rename(oldTextureFile, newTextureFile); } 
				catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}				
				readFile["albedoPath"] = newFileName;
			}
			if (readFile["normalAssigned"].get<bool>()) {
				std::string oldFileName = readFile["normalPath"].get<std::string>();
				std::string newFileName = finalName + "_normal.tif";
				fs::path oldTextureFile = materialDir / fs::path(oldFileName);
				fs::path newTextureFile = materialDir / fs::path(newFileName);
				try { fs::rename(oldTextureFile, newTextureFile);}
				catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}
				readFile["normalPath"] = newFileName;
			}
			if (readFile["roughAssigned"].get<bool>()) {
				std::string oldFileName = readFile["roughPath"].get<std::string>();
				std::string newFileName = finalName + "_roughness.tif";
				fs::path oldTextureFile = materialDir / fs::path(oldFileName);
				fs::path newTextureFile = materialDir / fs::path(newFileName);
				try { fs::rename(oldTextureFile, newTextureFile);}
				catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}
				readFile["roughPath"] = newFileName;
			}
			if (readFile["metalAssigned"].get<bool>()) {
				std::string oldFileName = readFile["metalPath"].get<std::string>();
				std::string newFileName = finalName + "_metallic.tif";
				fs::path oldTextureFile = materialDir / fs::path(oldFileName);
				fs::path newTextureFile = materialDir / fs::path(newFileName);
				try { fs::rename(oldTextureFile, newTextureFile);}
				catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what()); }
				readFile["metalPath"] = newFileName;
			}
			if (readFile["emissionAssigned"].get<bool>()) {
				std::string oldFileName = readFile["emissionPath"].get<std::string>();
				std::string newFileName = finalName + "_emissive.tif";
				fs::path oldTextureFile = materialDir / fs::path(oldFileName);
				fs::path newTextureFile = materialDir / fs::path(newFileName);
				try { fs::rename(oldTextureFile, newTextureFile);}
				catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}
				readFile["emissionPath"] = newFileName;
			}
			if (readFile["aoAssigned"].get<bool>()) {
				std::string oldFileName = readFile["aoPath"].get<std::string>();
				std::string newFileName = finalName + "_ambientOcclusion.tif";
				fs::path oldTextureFile = materialDir / fs::path(oldFileName);
				fs::path newTextureFile = materialDir / fs::path(newFileName);
				try {fs::rename(oldTextureFile, newTextureFile);}
				catch (fs::filesystem_error e) {MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}
				readFile["aoPath"] = newFileName;
			}

			// change name of thumbnail file
			std::string oldFileName = readFile["thumbnailPath"].get<std::string>();
			std::string newFileName = finalName + "_thumbnail.ktx2";
			fs::path oldTextureFile = materialDir / fs::path(oldFileName);
			fs::path newTextureFile = materialDir / fs::path(newFileName);
			try { fs::rename(oldTextureFile, newTextureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what()); }
			readFile["thumbnailPath"] = newFileName;

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
			mat->Name = readFile["name"].get<std::string>();
			mat->Albedo.r = readFile["albedo_r"].get<float>();
			mat->Albedo.g = readFile["albedo_g"].get<float>();
			mat->Albedo.b = readFile["albedo_b"].get<float>();
			mat->Roughness = readFile["roughness"].get<float>();
			mat->Metallic = readFile["metallic"].get<float>();

			mat->Emission.r = readFile["emission_r"].get<float>();
			mat->Emission.g = readFile["emission_g"].get<float>();
			mat->Emission.b = readFile["emission_b"].get<float>();
			mat->EmissionStrength = readFile["emissionStrength"].get<float>();

			mat->NormalStrength = readFile["normalStrength"].get<float>();

			mat->UVTiling.x = readFile["uvScale_x"].get<float>();
			mat->UVTiling.y = readFile["uvScale_y"].get<float>();
		
			bool albedoTexture		= readFile["albedoAssigned"].get<bool>();
			bool roughnessTexture	= readFile["roughAssigned"].get<bool>();
			bool metalTexture		= readFile["metalAssigned"].get<bool>();
			bool emissionTexture	= readFile["emissionAssigned"].get<bool>();
			bool normalTexture		= readFile["normalAssigned"].get<bool>();
			bool aoTexture			= readFile["aoAssigned"].get<bool>();

			if (albedoTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile["albedoPath"].get<std::string>();
				graphics::Texture* albedoTex = new graphics::Texture();

				albedoTex->generateFromFile(path.c_str(), true, true);
				//albedoTex->LoadFromKtx(path.c_str());
				mat->assignTexture(graphics::PBRTextureType::ALBEDO, albedoTex);
			}
			if (roughnessTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile["roughPath"].get<std::string>();
				graphics::Texture* roughnessTex = new graphics::Texture();
				
				roughnessTex->generateFromFile(path.c_str(), true, true);
				//roughnessTex->LoadFromKtx(path.c_str());
				mat->assignTexture(graphics::PBRTextureType::ROUGHNESS, roughnessTex);
			}
			if (metalTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile["metalPath"].get<std::string>();
				graphics::Texture* metallicTex = new graphics::Texture();
				metallicTex->generateFromFile(path.c_str(), true, true);
				//metallicTex->LoadFromKtx(path.c_str());
				mat->assignTexture(graphics::PBRTextureType::METALLIC, metallicTex);
			}
			if (emissionTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile["emissionPath"].get<std::string>();
				graphics::Texture* emissionTex = new graphics::Texture();
				emissionTex->generateFromFile(path.c_str(), true, true);
				//emissionTex->LoadFromKtx(path.c_str());
				mat->assignTexture(graphics::PBRTextureType::EMISSION, emissionTex);
			}
			if (normalTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile["normalPath"].get<std::string>();
				graphics::Texture* normalTex = new graphics::Texture();
				normalTex->generateFromFile(path.c_str(), true, true);
				//normalTex->LoadFromKtx(path.c_str());
				mat->assignTexture(graphics::PBRTextureType::NORMAL, normalTex);

				int normalFormat = readFile["normalMapFormat"].get<int>();
				if (normalFormat == 0) {
					mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGl);
				}
				else if (normalFormat == 1) {
					mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_DIRECTX);
				}
			}
			if (aoTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile["aoPath"].get<std::string>();
				graphics::Texture* aoTex = new graphics::Texture();
				aoTex->generateFromFile(path.c_str(), true, true);
				//aoTex->LoadFromKtx(path.c_str());
				mat->assignTexture(graphics::PBRTextureType::AMBIENTOCCLUSION, aoTex);
			}
			// load thumbnail

			dataFileStream.close();
		}

		m_userMaterialBound = true;
		m_activeMaterialDataFilePath = dataFile;
		m_activeMaterialID = materialInfo.runtime_ID;
		m_folderNodeOfActiveMaterial = parentNode;

		MnemosyEngine::GetInstance().GetScene().SetMaterial(mat);
	}

	void MaterialLibraryRegistry::SaveActiveMaterialToFile() {

		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();
		fs::path thumbnailPath = fs::path(activeMat.Name + "_thumbnail.ktx2");
				
		{ // Render thumbnail of active material
			fs::path libDir = m_fileDirectories.GetLibraryDirectoryPath();
			fs::path thumbnailAbsolutePath = libDir / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name) / thumbnailPath;
			MnemosyEngine::GetInstance().GetThumbnailManager().RenderThumbnailOfActiveMaterial(thumbnailAbsolutePath,m_selectedFolderNode, m_activeMaterialID);
									
		}
		
		std::ofstream dataFileStream;
		dataFileStream.open(m_activeMaterialDataFilePath.generic_string());		
		
		json MaterialJson; // top level json object
		MaterialJson["1_Mnemosy_Data_File"] = "Material Meta Data";

		MaterialJson["name"] = activeMat.Name;
		
		MaterialJson["albedo_r"] = activeMat.Albedo.r; // maybe convert from range 0-255 to 0-1 range by deviding by 255
		MaterialJson["albedo_g"] = activeMat.Albedo.g;
		MaterialJson["albedo_b"] = activeMat.Albedo.b;

		MaterialJson["roughness"] = activeMat.Roughness;
		MaterialJson["metallic"] = activeMat.Metallic;

		MaterialJson["emission_r"] = activeMat.Emission.r;
		MaterialJson["emission_g"] = activeMat.Emission.r;
		MaterialJson["emission_b"] = activeMat.Emission.r;
		MaterialJson["emissionStrength"] = activeMat.EmissionStrength;

		MaterialJson["normalStrength"] = activeMat.NormalStrength;
		MaterialJson["normalMapFormat"] = activeMat.GetNormalFormatAsInt(); // 0 = OpenGl, 1 = DirectX
		MaterialJson["uvScale_x"] = activeMat.UVTiling.x;
		MaterialJson["uvScale_y"] = activeMat.UVTiling.y;

		bool albedoAssigned		= activeMat.isAlbedoAssigned();
		bool roughAssigned		= activeMat.isRoughnessAssigned();
		bool metalAssigned		= activeMat.isMetallicAssigned();
		bool emissionAssigned	= activeMat.isEmissiveAssigned();
		bool normalAssigned		= activeMat.isNormalAssigned();
		bool aoAssigned			= activeMat.isAoAssigned();
		
		std::string albedoPath		= "notAssigned";
		std::string roughnessPath	= "notAssigned";
		std::string metallicPath	= "notAssigned";
		std::string emissionPath	= "notAssigned";
		std::string normalPath		= "notAssigned";
		std::string aoPath			= "notAssigned";

		std::string matName = activeMat.Name;

		if (albedoAssigned)		{ albedoPath	= matName	+ "_albedo.tif"; }
		if (roughAssigned)		{ roughnessPath = matName	+ "_roughness.tif"; }
		if (metalAssigned)		{ metallicPath	= matName	+ "_metallic.tif"; }
		if (emissionAssigned)	{ emissionPath	= matName	+ "_emissive.tif"; }
		if (normalAssigned)		{ normalPath	= matName	+ "_normal.tif"; }
		if (aoAssigned)			{ aoPath		= matName	+ "_ambientOcclusion.tif"; }

		MaterialJson["albedoAssigned"]		= albedoAssigned;
		MaterialJson["roughAssigned"]		= roughAssigned;
		MaterialJson["metalAssigned"]		= metalAssigned;
		MaterialJson["emissionAssigned"]	= emissionAssigned;
		MaterialJson["normalAssigned"]		= normalAssigned;
		MaterialJson["aoAssigned"]			= aoAssigned;

		MaterialJson["albedoPath"]		= albedoPath;
		MaterialJson["roughPath"]		= roughnessPath;
		MaterialJson["metalPath"]		= metallicPath;
		MaterialJson["emissionPath"]	= emissionPath;
		MaterialJson["normalPath"]		= normalPath;
		MaterialJson["aoPath"]			= aoPath;

		MaterialJson["thumbnailPath"] = thumbnailPath.generic_string();
				
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
		graphics::KtxImage ktxImg;

		if (textureType == graphics::ALBEDO) {
			
			filename = activeMat.Name + "_albedo.tif";
			fs::path exportPath = materialDir / fs::path(filename);
			exportManager.ExportMaterialTexturePngOrTif(exportPath, *tex, false, false);


			//bool success = ktxImg.ExportGlTexture(exportPath.generic_string().c_str(), tex->GetID(), tex->GetChannelsAmount(), tex->GetWidth(), tex->GetHeight(), graphics::MNSY_COLOR, true);
			//if (!success) {
			//	MNEMOSY_WARN("Unexpected error when trying to export image to ktx2 format: \nFilepath {}", filepath);
			//	activeMat.removeTexture(graphics::ALBEDO);
			//	delete tex;
			//	return;
			//}

		}
		else if (textureType == graphics::NORMAL) {
			filename = activeMat.Name + "_normal.tif";
			fs::path exportPath = materialDir / fs::path(filename);
			exportManager.ExportMaterialTexturePngOrTif(exportPath, *tex, false, true);
		}
		else if (textureType == graphics::ROUGHNESS) {
			filename = activeMat.Name + "_roughness.tif";
			fs::path exportPath = materialDir / fs::path(filename);
			exportManager.ExportMaterialTexturePngOrTif(exportPath, *tex, true, true);
		}
		else if (textureType == graphics::METALLIC) {
			filename = activeMat.Name + "_metallic.tif";
			fs::path exportPath = materialDir / fs::path(filename);
			exportManager.ExportMaterialTexturePngOrTif(exportPath, *tex, true, true);
		}
		else if (textureType == graphics::AMBIENTOCCLUSION) {
			filename = activeMat.Name + "_ambientOcclusion.tif";
			fs::path exportPath = materialDir / fs::path(filename);
			exportManager.ExportMaterialTexturePngOrTif(exportPath, *tex, true, true);
		}
		else if (textureType == graphics::EMISSION) {
			filename = activeMat.Name + "_emissive.tif";
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
		
		if (textureType == graphics::ALBEDO) {
			outFile["albedoAssigned"] = true;
			outFile["albedoPath"] = filename;
		}
		else if (textureType == graphics::NORMAL){
			outFile["normalAssigned"] = true;
			outFile["normalPath"] = filename;
		}
		else if (textureType == graphics::ROUGHNESS) {
			outFile["roughAssigned"] = true;
			outFile["roughPath"] = filename;
		}
		else if (textureType == graphics::METALLIC) {
			outFile["metalAssigned"] = true;
			outFile["metalPath"] = filename;
		}
		else if (textureType == graphics::AMBIENTOCCLUSION) {
			outFile["aoAssigned"] = true;
			outFile["aoPath"] = filename;
		}
		else if (textureType == graphics::EMISSION) {
			outFile["emissionAssigned"] = true;
			outFile["emissionPath"] = filename;
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
		json outFile = readFile;

		std::ofstream outFileStream;
		outFileStream.open(m_activeMaterialDataFilePath.generic_string());

		fs::path materialDir = m_fileDirectories.GetLibraryDirectoryPath() / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name);

		if (textureType == graphics::ALBEDO) {
			fs::path textureFile = materialDir / fs::path(outFile["albedoPath"].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			outFile["albedoAssigned"] = false;
			outFile["albedoPath"] = "notAssigned";
		}
		else if (textureType == graphics::NORMAL) {
			fs::path textureFile = materialDir / fs::path(outFile["normalPath"].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			outFile["normalAssigned"] = false;
			outFile["normalPath"] = "notAssigned";
			outFile["normalMapFormat"] = 0;
		}
		else if (textureType == graphics::ROUGHNESS) {
			fs::path textureFile = materialDir / fs::path(outFile["roughPath"].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			outFile["roughAssigned"] = false;
			outFile["roughPath"] = "notAssigned";
		}
		else if (textureType == graphics::METALLIC) {
			fs::path textureFile = materialDir / fs::path(outFile["metalPath"].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			outFile["metalAssigned"] = false;
			outFile["metalPath"] = "notAssigned";
		}
		else if (textureType == graphics::AMBIENTOCCLUSION) {
			fs::path textureFile = materialDir / fs::path(outFile["aoPath"].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGl);
			outFile["aoAssigned"] = false;
			outFile["aoPath"] = "notAssigned";
		}
		else if (textureType == graphics::EMISSION) {
			fs::path textureFile = materialDir / fs::path(outFile["emissionPath"].get<std::string>());
			try { fs::remove(textureFile); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			outFile["emissionAssigned"] = false;
			outFile["emissionPath"] = "notAssigned";
		}

		if (prettyPrintDataFile)
			outFileStream << outFile.dump(4);
		else
			outFileStream << outFile.dump(-1);
		outFileStream.close();
		
		activeMat.removeTexture(textureType);
	}

	void MaterialLibraryRegistry::OpenFolderNode(FolderNode* node){

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
			fs::path p = materialFolder / fs::path(activeMat.Name + "_albedo.tif");
			paths.push_back(p.generic_string());
			//MNEMOSY_TRACE("AddFilepath: {}", p.generic_string());
		}
		if (activeMat.isNormalAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + "_normal.tif");
			paths.push_back(p.generic_string());
			//MNEMOSY_TRACE("AddFilepath: {}", p.generic_string());
		}
		if (activeMat.isRoughnessAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + "_roughness.tif");
			paths.push_back(p.generic_string());
			//MNEMOSY_TRACE("AddFilepath: {}", p.generic_string());
		}
		if (activeMat.isMetallicAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + "_metallic.tif");
			paths.push_back(p.generic_string());
			//MNEMOSY_TRACE("AddFilepath: {}", p.generic_string());
		}
		if (activeMat.isAoAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + "_ambientOcclusion.tif");
			paths.push_back(p.generic_string());
			//MNEMOSY_TRACE("AddFilepath: {}", p.generic_string());
		}
		if (activeMat.isEmissiveAssigned()) {
			fs::path p = materialFolder / fs::path(activeMat.Name + "_emissive.tif");
			paths.push_back(p.generic_string());
			//MNEMOSY_TRACE("AddFilepath: {}", p.generic_string());
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
		MaterialJson["1_Mnemosy_Data_File"] = "Material Meta Data";
		MaterialJson["name"]		= name;

		MaterialJson["albedo_r"]	= 0.8f;
		MaterialJson["albedo_g"]	= 0.8f;
		MaterialJson["albedo_b"]	= 0.8f;

		MaterialJson["roughness"]	= 0.5f;
		MaterialJson["metallic"]	= 0.0f;

		MaterialJson["emission_r"]	= 0.0f;
		MaterialJson["emission_g"]	= 0.0f;
		MaterialJson["emission_b"]	= 0.0f;
		MaterialJson["emissionStrength"]	= 0.0f;

		MaterialJson["normalStrength"]		= 1.0f;
		MaterialJson["normalMapFormat"]		= 0; // 0 = OpenGl, 1 = DirectX

		MaterialJson["uvScale_x"]	= 1.0f;
		MaterialJson["uvScale_y"]	= 1.0f;

		
		MaterialJson["albedoAssigned"]		= false;
		MaterialJson["roughAssigned"]		= false;
		MaterialJson["metalAssigned"]		= false;
		MaterialJson["emissionAssigned"]	= false;
		MaterialJson["normalAssigned"]		= false;
		MaterialJson["aoAssigned"]			= false;
		
		MaterialJson["albedoPath"]		= "notAssigned";
		MaterialJson["roughPath"]		= "notAssigned";
		MaterialJson["metalPath"]		= "notAssigned";
		MaterialJson["emissionPath"]	= "notAssigned";
		MaterialJson["normalPath"]		= "notAssigned";
		MaterialJson["aoPath"]			= "notAssigned";

		fs::path thumbnailPath = fs::path(name + "_thumbnail.ktx2");
		MaterialJson["thumbnailPath"] = thumbnailPath.generic_string();


		if (prettyPrintDataFile)
			dataFileStream << MaterialJson.dump(4);
		else
			dataFileStream << MaterialJson.dump(-1);

		dataFileStream.close();
	}

	void MaterialLibraryRegistry::CreateDirectoryForNode(FolderNode* node) {

		mnemosy::core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();
		fs::path libraryDir = fd.GetLibraryDirectoryPath();

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