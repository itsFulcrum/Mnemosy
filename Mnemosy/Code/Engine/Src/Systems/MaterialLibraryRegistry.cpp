#include "Include/Systems/MaterialLibraryRegistry.h"

#include "Include/Core/Log.h"
#include "Include/MnemosyEngine.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Utils/KtxImage.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/ThumbnailManager.h"

#include <glad/glad.h>
#include <fstream>


namespace mnemosy::systems
{
	// == public methods
 
	MaterialLibraryRegistry::MaterialLibraryRegistry() 
		: m_fileDirectories{ MnemosyEngine::GetInstance().GetFileDirectories()}
	{

		m_runtimeIDCounter = 1;
		m_runtimeMaterialIDCounter = 1;
		//mnemosy::core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();
		fs::path pathToUserDirectoriesDataFile = m_fileDirectories.GetDataPath() / fs::path("UserLibraryDirectories.mnsydata");
		m_userDirectoriesDataFile = fs::directory_entry(pathToUserDirectoriesDataFile); 
		
		m_rootNodeName = "Root";
		m_folderNodeOfActiveMaterial = nullptr;
		LoadUserDirectoriesFromFile();
		m_selectedFolderNode = m_rootFolderNode;

	}

	MaterialLibraryRegistry::~MaterialLibraryRegistry() {
		
		SaveUserDirectoriesData();
		// recursivly cleanup folderTree heap memory
		RecursivCleanFolderTreeMemory(m_rootFolderNode);
	}

	
	void MaterialLibraryRegistry::RenameDirectory(FolderNode* node, std::string oldPathFromRoot) {
		
		//mnemosy::core::FileDirectories& fd = MnemosyEngine::GetInstance().GetFileDirectories();
		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();

		fs::path oldPath = libraryDir / fs::path(oldPathFromRoot);
		fs::path newPath = libraryDir / fs::path(node->pathFromRoot);

		try {
			fs::rename(oldPath, newPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::RenameDirectory: System error renaming directory: {} \nError message: {}", oldPath.generic_string(), newPath.generic_string(), error.what());
		}
		
		RecursivUpadtePathFromRoot(node);



		// check if the folder  we renamed  included the active material in its hierarchy
		if (!m_activeMaterialDataFilePath.empty()) {

			fs::directory_entry activeMaterialDataFile = fs::directory_entry(m_activeMaterialDataFilePath);
			if (!activeMaterialDataFile.exists()) {
				MNEMOSY_WARN("MaterialLibraryRegistry::RenameDirectory: active material path doesn exist anymore");
				std::string activeMatName = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial().Name;
				m_activeMaterialDataFilePath = libraryDir / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) /  fs::path(activeMatName) / fs::path(activeMatName + ".mnsydata");
			}
		}
	}

	void MaterialLibraryRegistry::MoveDirectory(FolderNode* dragSource, FolderNode* dragTarget) {

		fs::path libraryDir = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();
		fs::path fromPath = libraryDir / fs::path(dragSource->pathFromRoot);
		fs::path toPath = libraryDir / fs::path(dragTarget->pathFromRoot);

		// Copying and then removing works rn but is not very elegant. fs::rename() does not work and throws acces denied error
			//fs::rename(fromPath, toPath / fromPath.filename());

		try { // copy directory to new location
			fs::copy(fromPath, toPath / fromPath.filename(), fs::copy_options::recursive);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveDirectory: System Error Copying directory: \nMessage: {}", error.what());
			return;
		}
		try { // remove old directory
			fs::remove_all(fromPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveDirectory: System Error Removing old directory: \nMessage: {}", error.what());
			return;
		}
		
		// Updating Internal Data
		// removing from old parent
		for (int i = 0; i < dragSource->parent->subNodes.size(); i++) {
			if (dragSource->parent->subNodes[i]->name == dragSource->name) {
				dragSource->parent->subNodes.erase(dragSource->parent->subNodes.begin() + i);
			}
		}
		dragSource->parent = dragTarget; // set new parent
		dragTarget->subNodes.push_back(dragSource); // enlist into target subNodes
		// update path from root
		if (dragSource->parent->name == m_rootNodeName) {
			dragSource->pathFromRoot = dragSource->name; 
		}
		else {
			dragSource->pathFromRoot = dragSource->parent->pathFromRoot + "/" + dragSource->name; 
		}

		// should prob save to data
		SaveUserDirectoriesData();



		// check if the folder  we renamed  included the active material in its hierarchy
		fs::directory_entry activeMaterialDataFile = fs::directory_entry(m_activeMaterialDataFilePath);
		if (!activeMaterialDataFile.exists()) {
			MNEMOSY_WARN("MaterialLibraryRegistry::MoveDirectory: active material path doesnt exist anymore");
			std::string activeMatName = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial().Name;
			m_activeMaterialDataFilePath = libraryDir / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMatName) / fs::path(activeMatName + ".mnsydata");
		}


	}

	void MaterialLibraryRegistry::DeleteFolderHierarchy(FolderNode* node) {

		// never delete root node
		if (node->parent == nullptr || node->name == m_rootNodeName) {
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

		// remove node from parent subnodes list
		for (int i = 0; i < node->parent->subNodes.size(); i++) {
			if (node->parent->subNodes[i]->name == node->name) {
				node->parent->subNodes.erase(node->parent->subNodes.begin() + i);
			}
		}

		// Free memory for all subsequent nodes recursivly
		RecursivCleanFolderTreeMemory(node);

		// check if the folder hierarchy we deleted included the active material
		fs::directory_entry activeMaterialDataFile = fs::directory_entry(m_activeMaterialDataFilePath);
		if (!activeMaterialDataFile.exists()) {
			SetDefaultMaterial();
		}

		
	}


	FolderNode* MaterialLibraryRegistry::CreateFolderNode(FolderNode* parentNode, std::string name) {
		FolderNode* node = new FolderNode();
		node->name = name;
		node->parent = parentNode;

		std::string pathFromRoot = "";
		if (parentNode != nullptr) { // if this not root node
			if (parentNode->name == m_rootNodeName) { // if parent is root
				pathFromRoot = node->name;
			}
			else {
				pathFromRoot = parentNode->pathFromRoot + "/" + node->name;
			}
		}
		node->pathFromRoot = pathFromRoot;

		node->runtime_ID = m_runtimeIDCounter;
		m_runtimeIDCounter++;
		// create system folder 
		CreateDirectoryForNode(node);
		
		// we can NOT Call SaveUserDirectoriesData() here because we call this function when initialising and reading from the file..
		return node;
	}

	FolderNode* MaterialLibraryRegistry::GetRootFolder() {

		return m_rootFolderNode;
	}

	FolderNode* MaterialLibraryRegistry::RecursivGetNodeByRuntimeID(FolderNode* node, unsigned int id) {

		if (node == nullptr) {
			return nullptr;
		}

		if (node->runtime_ID == id) {
			return node;
		}

		FolderNode* foundNode = nullptr;
		for (FolderNode* child : node->subNodes) {

			foundNode = RecursivGetNodeByRuntimeID(child, id);
			if (foundNode != nullptr) {
				break; // Found the node, no need to search further
			}
		}

		return foundNode;
	}

	void MaterialLibraryRegistry::CreateNewMaterial(FolderNode* node, std::string name) {

		// check if name exists already and add suffix number if needed
		int suffix = 1;
		std::string finalName = name;
		while (node->SubMaterialExistsAlready(finalName)) {
			finalName = name + "_" + std::to_string(suffix);
			suffix++;
		}

		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();
		// create directory for material
		fs::path materialDirectory = libraryDir / fs::path(node->pathFromRoot) / fs::path(finalName);
		//MNEMOSY_TRACE("Add Material: new material directory: {}", materialDirectory.generic_string());

		try {
			fs::create_directory(materialDirectory);
		} catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::AddMaterial: System error creating directory. \nError message: {}", error.what());
			return;
		}
		
		// create default material data file

		// Copy Default thumbnail image
		fs::path pathToDefaultThumbnail = m_fileDirectories.GetTexturesPath() / fs::path("default_thumbnail.ktx2");
		fs::path pathToMaterialThumbnail = materialDirectory / fs::path(finalName + "_thumbnail.ktx2");
		fs::copy_file(pathToDefaultThumbnail,pathToMaterialThumbnail);
						
		CreateNewMaterialDataFile(materialDirectory,finalName);

		// adding entry to list of directory node;
		MaterialInfo matInfo;
		matInfo.name = finalName; 
		matInfo.runtime_ID = m_runtimeMaterialIDCounter;
		m_runtimeMaterialIDCounter++;
		matInfo.thumbnailTexure_ID = 0;
		matInfo.thumbnailLoaded = false;

		// check if it was created in the currently opend folder
		if (node == m_selectedFolderNode) {
			// make sure it gets loaded
			MnemosyEngine::GetInstance().GetThumbnailManager().NewThumbnailInActiveFolder();
		}

		node->subMaterials.push_back(matInfo);

		SaveUserDirectoriesData();
	}

	void MaterialLibraryRegistry::ChangeMaterialName(systems::FolderNode* node, systems::MaterialInfo& materialInfo, std::string& newName, int positionInVector) {

		std::string oldName = materialInfo.name;
		unsigned int matID = materialInfo.runtime_ID;

		if (materialInfo.name == newName)
			return;
		
		std::string finalName = newName;
		// check if name exists already add suffix number if needed
		{
			int suffix = 1;
			while (node->SubMaterialExistsAlready(finalName)) {
				finalName = newName + "_" + std::to_string(suffix);
				suffix++;
			}
		}

		fs::path libraryDir = m_fileDirectories.GetLibraryDirectoryPath();

		// change name of thumbnail img


		fs::path materialDir = libraryDir / fs::path(node->pathFromRoot) / fs::path(oldName);

		// change name of data file. TODO: and adjust all data inside
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
			
			
			// Renaming all accosiated textures
			if (readFile["albedoAssigned"].get<bool>()) {
				std::string oldFileName = readFile["albedoPath"].get<std::string>();
				std::string newFileName = finalName + "_albedo.ktx2";
				fs::path oldTextureFile = materialDir / fs::path(oldFileName);
				fs::path newTextureFile = materialDir / fs::path(newFileName);				
				try { fs::rename(oldTextureFile, newTextureFile); } 
				catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}				
				readFile["albedoPath"] = newFileName;
			}
			if (readFile["normalAssigned"].get<bool>()) {
				std::string oldFileName = readFile["normalPath"].get<std::string>();
				std::string newFileName = finalName + "_normal.ktx2";
				fs::path oldTextureFile = materialDir / fs::path(oldFileName);
				fs::path newTextureFile = materialDir / fs::path(newFileName);
				try { fs::rename(oldTextureFile, newTextureFile);}
				catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}
				readFile["normalPath"] = newFileName;
			}
			if (readFile["roughAssigned"].get<bool>()) {
				std::string oldFileName = readFile["roughPath"].get<std::string>();
				std::string newFileName = finalName + "_roughness.ktx2";
				fs::path oldTextureFile = materialDir / fs::path(oldFileName);
				fs::path newTextureFile = materialDir / fs::path(newFileName);
				try { fs::rename(oldTextureFile, newTextureFile);}
				catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}
				readFile["roughPath"] = newFileName;
			}
			if (readFile["metalAssigned"].get<bool>()) {
				std::string oldFileName = readFile["metalPath"].get<std::string>();
				std::string newFileName = finalName + "_metallic.ktx2";
				fs::path oldTextureFile = materialDir / fs::path(oldFileName);
				fs::path newTextureFile = materialDir / fs::path(newFileName);
				try { fs::rename(oldTextureFile, newTextureFile);}
				catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what()); }
				readFile["metalPath"] = newFileName;
			}
			if (readFile["emissionAssigned"].get<bool>()) {
				std::string oldFileName = readFile["emissionPath"].get<std::string>();
				std::string newFileName = finalName + "_emissive.ktx2";
				fs::path oldTextureFile = materialDir / fs::path(oldFileName);
				fs::path newTextureFile = materialDir / fs::path(newFileName);
				try { fs::rename(oldTextureFile, newTextureFile);}
				catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry:ChangeMaterialName: System error renaming file. \nError message: {}", e.what());}
				readFile["emissionPath"] = newFileName;
			}
			if (readFile["aoAssigned"].get<bool>()) {
				std::string oldFileName = readFile["aoPath"].get<std::string>();
				std::string newFileName = finalName + "_ambientOcclusion.ktx2";
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

		// change name in vector of names..
		node->subMaterials[positionInVector].name = finalName;

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

		//TODO:
		// once i have a selected material that is displayed in the viewport and in material editor,
		// I should switch it to a different one here and free memory;
		
		// deleting from vector
		node->subMaterials.erase(node->subMaterials.begin() + positionInVector);
		

		// check if we are delete the active material
		if (m_activeMaterialID == matID) {
			SetDefaultMaterial();
		}

		// maybe save library data file
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

		

		// enlist into target node list

		// remove from source node list
		// this could maybe be done faster if we pass the vector position directly into the function
		for (int i = 0; i < sourceNode->subMaterials.size(); i++) {
			if (sourceNode->subMaterials[i].name == materialName) {

				// make sure thumbnail get popperly unloaded
				if (sourceNode == m_selectedFolderNode) {
					MnemosyEngine::GetInstance().GetThumbnailManager().DeleteThumbnailFromCache(sourceNode->subMaterials[i]);
				}


				// enlist into target node list
				targetNode->subMaterials.push_back(sourceNode->subMaterials[i]);
				// remove from source nodel list
				sourceNode->subMaterials.erase(sourceNode->subMaterials.begin() + i);

				break;
			}
		}


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
				albedoTex->LoadFromKtx(path.c_str());
				mat->assignTexture(graphics::PBRTextureType::ALBEDO, albedoTex);
			}
			if (roughnessTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile["roughPath"].get<std::string>();
				graphics::Texture* roughnessTex = new graphics::Texture();
				roughnessTex->LoadFromKtx(path.c_str());
				mat->assignTexture(graphics::PBRTextureType::ROUGHNESS, roughnessTex);
			}
			if (metalTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile["metalPath"].get<std::string>();
				graphics::Texture* metallicTex = new graphics::Texture();
				metallicTex->LoadFromKtx(path.c_str());
				mat->assignTexture(graphics::PBRTextureType::METALLIC, metallicTex);
			}
			if (emissionTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile["emissionPath"].get<std::string>();
				graphics::Texture* emissionTex = new graphics::Texture();
				emissionTex->LoadFromKtx(path.c_str());
				mat->assignTexture(graphics::PBRTextureType::EMISSION, emissionTex);
			}
			if (normalTexture) {
				std::string path = materialDir.generic_string() + "/" + readFile["normalPath"].get<std::string>();
				graphics::Texture* normalTex = new graphics::Texture();
				normalTex->LoadFromKtx(path.c_str());
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
				aoTex->LoadFromKtx(path.c_str());
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

		if (albedoAssigned)		{ albedoPath	= matName	+ "_albedo.ktx2"; }
		if (roughAssigned)		{ roughnessPath = matName	+ "_roughness.ktx2"; }
		if (metalAssigned)		{ metallicPath	= matName	+ "_metallic.ktx2"; }
		if (emissionAssigned)	{ emissionPath	= matName	+ "_emissive.ktx2"; }
		if (normalAssigned)		{ normalPath	= matName	+ "_normal.ktx2"; }
		if (aoAssigned)			{ aoPath		= matName	+ "_ambientOcclusion.ktx2"; }

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

		//fs::path thumbnailPath = fs::path(activeMat.Name + "_thumbnail.ktx2");
		MaterialJson["thumbnailPath"] = thumbnailPath.generic_string();

		if (prettyPrintDataFile)
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
		// save texture to disk with correct filename to folder of active material

		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();
		
		if (!UserMaterialBound()) {
			graphics::Texture* tex = new graphics::Texture();
			tex->generateFromFile(filepath.c_str(), true, true);

			activeMat.assignTexture(textureType, tex);
			return;
		}

		fs::path materialDir = m_fileDirectories.GetLibraryDirectoryPath() / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name) ;
		
		
		graphics::Texture* tex = new graphics::Texture();
		tex->generateFromFile(filepath.c_str(),true,true);

		std::string filename;
		graphics::KtxImage ktxImg;
		if (textureType == graphics::ALBEDO) {
			
			filename = activeMat.Name + "_albedo.ktx2";
			fs::path exportPath = materialDir / fs::path(filename);
			ktxImg.ExportGlTexture(exportPath.generic_string().c_str(), tex->GetID(), tex->GetChannelsAmount(), tex->GetWidth(), tex->GetHeight(), graphics::MNSY_COLOR, true);
		}
		else if (textureType == graphics::NORMAL) {
			filename = activeMat.Name + "_normal.ktx2";
			fs::path exportPath = materialDir / fs::path(filename);
			ktxImg.ExportGlTexture(exportPath.generic_string().c_str(), tex->GetID(), tex->GetChannelsAmount(), tex->GetWidth(), tex->GetHeight(), graphics::MNSY_NORMAL, true);
		}
		else if (textureType == graphics::ROUGHNESS) {
			filename = activeMat.Name + "_roughness.ktx2";
			fs::path exportPath = materialDir / fs::path(filename);
			ktxImg.ExportGlTexture(exportPath.generic_string().c_str(), tex->GetID(), tex->GetChannelsAmount(), tex->GetWidth(), tex->GetHeight(), graphics::MNSY_LINEAR_CHANNEL, true);
		}
		else if (textureType == graphics::METALLIC) {
			filename = activeMat.Name + "_metallic.ktx2";
			fs::path exportPath = materialDir / fs::path(filename);
			ktxImg.ExportGlTexture(exportPath.generic_string().c_str(), tex->GetID(), tex->GetChannelsAmount(), tex->GetWidth(), tex->GetHeight(), graphics::MNSY_LINEAR_CHANNEL, true);
		}
		else if (textureType == graphics::AMBIENTOCCLUSION) {
			filename = activeMat.Name + "_ambientOcclusion.ktx2";
			fs::path exportPath = materialDir / fs::path(filename);
			ktxImg.ExportGlTexture(exportPath.generic_string().c_str(), tex->GetID(), tex->GetChannelsAmount(), tex->GetWidth(), tex->GetHeight(), graphics::MNSY_LINEAR_CHANNEL, true);
		}
		else if (textureType == graphics::EMISSION) {
			filename = activeMat.Name + "_emissive.ktx2";
			fs::path exportPath = materialDir / fs::path(filename);
			ktxImg.ExportGlTexture(exportPath.generic_string().c_str(), tex->GetID(), tex->GetChannelsAmount(), tex->GetWidth(), tex->GetHeight(), graphics::MNSY_COLOR, true);
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
			MNEMOSY_WARN("Node is nullptr");
			return;
		}

		// already selectd node
		if (m_selectedFolderNode == node) {
			return;
		}
		
		if (m_selectedFolderNode == nullptr) {
			
			m_selectedFolderNode = node;
		}

		MnemosyEngine::GetInstance().GetThumbnailManager().DeleteLoadedThumbnailsOfActiveFolder(m_selectedFolderNode);


		m_selectedFolderNode = node;

		if (!node->HasMaterials()) {
			return;
		}

		// load thumbnails of the opend folder

		FolderNode* selectedNode = m_selectedFolderNode;


		return;
		//fs::path folderDir = m_fileDirectories.GetLibraryDirectoryPath() / fs::path(selectedNode->pathFromRoot);
		//MnemosyEngine::GetInstance().GetThumbnailManager().LoadThumbnailsOfActiveFolder(m_selectedFolderNode, folderDir);





	}

	void MaterialLibraryRegistry::ClearUserMaterialsAndFolders() {

		// Clearing all User materials and folders from memory but not deleting any files.
		// root node stays in takt
		SetDefaultMaterial();
		m_selectedFolderNode = m_rootFolderNode;

		if (!m_rootFolderNode->IsLeafNode()) {

			for (int i = 0; i < m_rootFolderNode->subNodes.size(); i++) {
			
				RecursivCleanFolderTreeMemory(m_rootFolderNode->subNodes[i]);
			}
			m_rootFolderNode->subNodes.clear();
		}
		if (m_rootFolderNode->HasMaterials()) {
			m_rootFolderNode->subMaterials.clear();
		}
		
		SaveUserDirectoriesData();
	}

	
	
	void MaterialLibraryRegistry::LoadUserDirectoriesFromFile() {

		bool fileExists = CheckDataFile(m_userDirectoriesDataFile);
		if (!fileExists) {
			// if file does not exist we explicitly create root node here otherwise it should be created from within the file
			m_rootFolderNode = new FolderNode();
			m_rootFolderNode->name = m_rootNodeName;
			m_rootFolderNode->parent = nullptr;
			m_rootFolderNode->runtime_ID = m_runtimeIDCounter;
			m_runtimeIDCounter++;
		}

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
		
		m_rootNodeName = readFile["3_UserDirectories"][m_rootNodeName]["1_name"].get<std::string>();
		bool rootIsLeaf = readFile["3_UserDirectories"][m_rootNodeName]["2_isLeaf"].get<bool>();

		FolderNode* rootFolder = CreateFolderNode(nullptr, m_rootNodeName);

		json rootJson = readFile["3_UserDirectories"][m_rootNodeName];

		m_rootFolderNode = rootFolder; 


		RecursivLoadDirectories(rootFolder, rootJson);

		dataFileStream.close();

	}

	void MaterialLibraryRegistry::SaveUserDirectoriesData() {

		std::string pathToDataFileString = m_userDirectoriesDataFile.path().generic_string();
		if (CheckDataFile(m_userDirectoriesDataFile)) {
			// if file exists we clear it first; -> really shoudl think if this is a good idea though
			std::ofstream file;
			file.open(pathToDataFileString);
			file << "";
			file.close();
		}

		std::ofstream dataFileStream;
		// start Saving
		dataFileStream.open(pathToDataFileString);
		json LibraryDirectoriesJson; // top level json object
		LibraryDirectoriesJson["1_Mnemosy_Data_File"] = "UserLibraryDirectoriesData";

		json HeaderInfo;
		std::string descriptionString = "This file stores the treelike folder structure defined by users to organise their materials";
		HeaderInfo["Description"] = descriptionString;

		LibraryDirectoriesJson["2_Header_Info"] = HeaderInfo;

		json userDirectoriesJson;

		json rootFolderJson = RecursivSaveDirectories(m_rootFolderNode);

		userDirectoriesJson[m_rootNodeName] = rootFolderJson;

		
		LibraryDirectoriesJson["3_UserDirectories"] = userDirectoriesJson;


		
		if (prettyPrintDataFile)
			dataFileStream << LibraryDirectoriesJson.dump(4);
		else
			dataFileStream << LibraryDirectoriesJson.dump(-1);

		dataFileStream.close();
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

	void MaterialLibraryRegistry::RecursivUpadtePathFromRoot(FolderNode* node) {

		if (node->parent->IsRoot()) {
			node->pathFromRoot = node->name;
		}
		else {
			node->pathFromRoot = node->parent->pathFromRoot + "/" + node->name;
		}

		if (!node->IsLeafNode()) {
			for (FolderNode* child : node->subNodes) {
				RecursivUpadtePathFromRoot(child);
			}
		}
	}

	void MaterialLibraryRegistry::RecursivCleanFolderTreeMemory(FolderNode* node) {


		if (!node->subNodes.empty()) {
			for (FolderNode* subNode : node->subNodes) {
				RecursivCleanFolderTreeMemory(subNode);
			}
			node->subNodes.clear();
		} 

		if (!node->subMaterials.empty()) {
			node->subMaterials.clear();
		}

		// if node we are deleting is the currently selected one, select root node.
		if(node == m_selectedFolderNode){
			OpenFolderNode(m_rootFolderNode);
		}

		delete node;
		node = nullptr;
	}

	void MaterialLibraryRegistry::RecursivLoadDirectories(FolderNode* node, json& jsonNode) {

		bool isLeafNode = jsonNode["2_isLeaf"].get<bool>();

		bool hasMaterials = jsonNode["6_hasMaterials"].get<bool>();
		if (hasMaterials) {
			std::vector<std::string> subMatNames = jsonNode["6_materialEntries"].get<std::vector<std::string>>();
			for (int i = 0; i < subMatNames.size(); i++) {
				
				MaterialInfo matInfo; 
				matInfo.name = subMatNames[i];
				matInfo.runtime_ID = m_runtimeMaterialIDCounter;
				m_runtimeMaterialIDCounter++;

				node->subMaterials.push_back(matInfo);
			}

		}

		if (!isLeafNode) {

			std::vector<std::string> subFolderNames = jsonNode["4_subFolderNames"].get<std::vector<std::string>>(); // 3 nature,wood,stone
						
			for (int i = 0; i < subFolderNames.size(); i++) {

				FolderNode* subNode = new FolderNode();
				subNode->parent = node;
				subNode->name = subFolderNames[i];
				subNode->pathFromRoot = jsonNode["5_subFolders"][subFolderNames[i]]["3_pathFromRoot"].get<std::string>();
				subNode->runtime_ID = m_runtimeIDCounter;
				m_runtimeIDCounter++;
				node->subNodes.push_back(subNode);
				
				json subJson = jsonNode["5_subFolders"][subFolderNames[i]];
				RecursivLoadDirectories(subNode, subJson);
				
			}
		}
	}

	json MaterialLibraryRegistry::RecursivSaveDirectories(FolderNode* node)
	{

		json nodeJson;

		nodeJson["1_name"] = node->name;
		bool isLeafNode = node->IsLeafNode();
		nodeJson["2_isLeaf"] = isLeafNode;


		std::string pathFromRoot = "";

		if (node->parent != nullptr) { // if not root node

			if (node->parent->name == m_rootNodeName) {

				pathFromRoot = node->name;
			}
			else {
				pathFromRoot = node->parent->pathFromRoot + "/" + node->name;
			}
		}
		//MNEMOSY_DEBUG("PathFromRoot: {}", pathFromRoot);
		nodeJson["3_pathFromRoot"] = pathFromRoot;


		bool hasMaterials = !node->subMaterials.empty();

		nodeJson["6_hasMaterials"] = hasMaterials;
		if (hasMaterials) {

			std::vector<std::string> matNames;
			for (int i = 0; i < node->subMaterials.size(); i++) {
				matNames.push_back(node->subMaterials[i].name);
			}

			nodeJson["6_materialEntries"] = matNames;
			matNames.clear();
		}

		if (!isLeafNode) {
			std::vector<std::string> subNodeNames;

			for (int i = 0; i < node->subNodes.size(); i++) {

				subNodeNames.push_back(node->subNodes[i]->name);
			}
			nodeJson["4_subFolderNames"] = subNodeNames;

			json subNodes;
			for (int i = 0; i < node->subNodes.size(); i++) {

				subNodes[node->subNodes[i]->name] = RecursivSaveDirectories(node->subNodes[i]);
			}

			nodeJson["5_subFolders"] = subNodes;

		}



		return nodeJson;
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