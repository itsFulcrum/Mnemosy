#include "Include/Systems/MaterialLibraryRegistry.h"

#include "Include/MnemosyEngine.h"
#include "Include/MnemosyConfig.h"
#include "Include/Core/Log.h"
#include "Include/Core/Clock.h"
#include "Include/Core/FileDirectories.h"
#include <json.hpp>

#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/TextureDefinitions.h"
#include "Include/Graphics/Utils/Picture.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/Utils/KtxImage.h"

#include "Include/Systems/LibraryProcedures.h"
#include "Include/Systems/JsonKeys.h"
#include "Include/Systems/ExportManager.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/FolderTree.h"
#include "Include/Systems/TextureGenerationManager.h"
#include "Include/Systems/ThumbnailManager.h"


#include <glad/glad.h>
#include <fstream>
#include <thread>

namespace mnemosy::systems {
	// == public methods

	void MaterialLibraryRegistry::Init()
	{
		m_folderTree = nullptr;
		inSearchMode = false;

		m_fileDirectories = nullptr;
		m_selectedFolderNode = nullptr;


		// data file 
		prettyPrintDataFile = false;
		prettyPrintMaterialFiles = false;

		m_lastActiveMaterialLibEntry = systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT;


		namespace fs = std::filesystem;

#ifdef MNEMOSY_CONFIG_DEBUG
		prettyPrintMaterialFiles = true;
#endif

		m_folderTree = arena_placement_new(FolderTree);
		m_folderTree->Init(jsonLibKey_RootNodeName);

		m_fileDirectories = &MnemosyEngine::GetInstance().GetFileDirectories();

		fs::path pathToUserDirectoriesDataFile = m_fileDirectories->GetUserLibDataFile();

		if (!LibProcedures::CheckDataFile(pathToUserDirectoriesDataFile)) {
			MNEMOSY_WARN("Failed to read materialLibraryData file: Creating new empty file at {}", pathToUserDirectoriesDataFile.generic_string());
			SaveUserDirectoriesData();
		}

		float timeStart = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();
		LoadUserDirectoriesFromFile();
		float timeEnd = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();


		unsigned int materialCount = m_folderTree->RecursiveCountMaterials(m_folderTree->GetRootPtr(), 0);

		MNEMOSY_DEBUG("Loaded Material Library in {} Seconds, Loaded {} Material Entries", timeEnd - timeStart, materialCount);


		m_selectedFolderNode = m_folderTree->GetRootPtr();
	}

	void MaterialLibraryRegistry::Shutdown() {
		SaveUserDirectoriesData();
		m_folderTree->Shutdown();
	}


	void MaterialLibraryRegistry::LoadUserDirectoriesFromFile() {

		std::filesystem::path materialLibraryDataFilePath = m_fileDirectories->GetUserLibDataFile();

		if (!LibProcedures::CheckDataFile(materialLibraryDataFilePath)) {

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
		dataFileStream.close();


		m_folderTree->LoadFromJson(readFile);

	}

  	void MaterialLibraryRegistry::SaveUserDirectoriesData() {
		namespace fs = std::filesystem;

		// this function throws warnings if we change the library directory.
		fs::path materialLibraryDataFilePath = m_fileDirectories->GetUserLibDataFile();

		if (LibProcedures::CheckDataFile(materialLibraryDataFilePath)) {

			// if data file exists we first make a backup copy
			fs::path copyTo = m_fileDirectories->GetDataPath() / fs::path("MnemosyMaterialLibraryData_LAST_BACKUP.mnsydata");
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

		LibProcedures::CreateDirectoryForFolderNode(node);

		// we can NOT Call SaveUserDirectoriesData() here because
		// we call this function when initialising and reading from the file..
		return node;
	}

	void MaterialLibraryRegistry::RenameFolder(FolderNode* node, std::string& newName) {

		namespace fs = std::filesystem;

		if (node->IsRoot()) {
			MNEMOSY_WARN("You can't change the name of the root directory");
			return;
		}

		// store old path because pathFromRoot is upadeted inside RenameFolder() method
		fs::path libraryDir = m_fileDirectories->GetLibraryDirectoryPath();
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
			MNEMOSY_ERROR("System error renaming directory: Maybe contents of the folder are opend in another process or program? {} \nError message: {}", oldPath.generic_string(), newPath.generic_string(), error.what());

			// revert name if system can't rename the file on disk
			m_folderTree->RenameFolder(node, oldName);
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
			std::vector<LibEntry*> entriesCopy = node->subEntries;

			for (size_t i = 0; i < entriesCopy.size(); i++) {

				LibEntry_Move(node, node->parent, entriesCopy[i]);
			}
			entriesCopy.clear();
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

		namespace fs = std::filesystem;

		// never delete root node
		if (node->IsRoot()) {
			MNEMOSY_WARN("You can't delete the root directory");
			return;
		}

		// check if the selected folder node is part of the hierarchy, if so select the folder above the node to delete
		if (m_selectedFolderNode) {

			if (m_folderTree->IsNodeWithinHierarchy(node, m_selectedFolderNode)) {

				OpenFolderNode(node->parent);
			}
		}

		// check if the currently selected material entry is part of the hierarchy and if so set default material 
		if (m_activeLibEntry) {

			if (m_folderTree->IsLibEntryWithinHierarchy(node,m_activeLibEntry)) {
				SetDefaultMaterial();
			}
		}

		{ // delete directories from disk

			fs::path folderPath = Folder_GetFullPath(node);
			try {
				// this call removes all files and directories underneith permanently without moving it to trashbin
				fs::remove_all(folderPath);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteFolderHierarchy: System error deleting directory: {} \nError Message: {} ", folderPath.generic_string(), error.what());
			}
		}

		//OpenFolderNode(node->parent);
		m_folderTree->DeleteFolderHierarchy(node);
	}


	// TODO: make sure thumbnails get directly created for the different entry types
	void MaterialLibraryRegistry::LibEntry_CreateNew(FolderNode* node, LibEntryType type , std::string& name) {

		if (type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {
			MNEMOSY_WARN("At the moment on SKYBOX Mat is not supported");
			return;
		}

		namespace fs = std::filesystem;

		LibEntry* libEntry = m_folderTree->CreateNewLibEntry(node, type, name);

		// create directory for entry
		fs::path entryDirectory = LibEntry_GetFolderPath(libEntry);

		try {
			fs::create_directory(entryDirectory);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::AddMaterial: System error creating directory. \nError message: {}", error.what());

			int posInVector = -1;
			for (size_t i = 0; i < node->subEntries.size(); i++) {

				if (node->subEntries[i]->runtime_ID == libEntry->runtime_ID) {
					posInVector = i;
				}
			}
			MNEMOSY_ASSERT(posInVector != -1, "This should not happen because we just added the material");


			m_folderTree->DeleteLibEntry(node, posInVector);

			return;
		}


		if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {

			// create default material data file

			// Copy Default thumbnail image
			fs::path pathToDefaultThumbnail = m_fileDirectories->GetTexturesPath() / fs::path("default_thumbnail.ktx2");
			fs::path pathToMaterialThumbnail = entryDirectory / fs::path(libEntry->name + texture_fileSuffix_thumbnail);

			try {
				fs::copy_file(pathToDefaultThumbnail, pathToMaterialThumbnail);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::AddMaterial: System error copyingFile. \nError message: {}", error.what());

				return;
			}

			LibProcedures::LibEntry_PbrMaterial_CreateNewDataFile(libEntry,prettyPrintMaterialFiles);
		}
		else if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {

			LibProcedures::LibEntry_UnlitMaterial_CreateNewDataFile(libEntry,prettyPrintMaterialFiles);

		}
		else if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

			LibProcedures::LibEntry_SkyboxMaterial_CreateNewDataFile(libEntry, prettyPrintMaterialFiles);
		}

		// check if it was created in the currently opend folder
		if (node == m_selectedFolderNode) {
			// make sure it gets loaded

			MnemosyEngine::GetInstance().GetThumbnailManager().AddLibEntryToActiveThumbnails(libEntry);
		}

		SaveUserDirectoriesData();
	}

	void MaterialLibraryRegistry::LibEntry_Rename(systems::LibEntry* libEntry, std::string& newName) {
		namespace fs = std::filesystem;

		std::string oldName = libEntry->name;
		unsigned int matID = libEntry->runtime_ID;

		if (libEntry->name == newName)
			return;


		fs::path entryFolderPathOld = LibEntry_GetFolderPath(libEntry);

		// rename entry internally
		m_folderTree->RenameLibEntry(libEntry, newName);


		// rename all files inside the folder 
		if (libEntry->type == LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {

			LibProcedures::LibEntry_PbrMaterial_RenameFiles(libEntry,entryFolderPathOld,oldName,prettyPrintMaterialFiles);
		}
		else if (libEntry->type == LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {

			LibProcedures::LibEntry_UnlitMaterial_RenameFiles(libEntry,entryFolderPathOld,oldName,prettyPrintMaterialFiles);
		}
		else if (libEntry->type == LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {
			LibProcedures::LibEntry_SkyboxMaterial_RenameFiles(libEntry,entryFolderPathOld,oldName,prettyPrintMaterialFiles);
		}

		// rename folder itself
		{
			fs::path newPath = LibEntry_GetFolderPath(libEntry);

			try { 
				fs::rename(entryFolderPathOld, newPath);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("System error renaming directory. \nError message: {}", error.what());
			}
		}

		// save library data file;
		SaveUserDirectoriesData();
	}

	
	void MaterialLibraryRegistry::LibEntry_Delete(systems::LibEntry* libEntry, int positionInVector) {

		namespace fs = std::filesystem;

		// check if entry is part of the opend folder
		if (libEntry->parent == m_selectedFolderNode) {
			// unload thumbnail
			MnemosyEngine::GetInstance().GetThumbnailManager().RemoveLibEntryFromActiveThumbnails(libEntry);
		}

		// Set default material if we are deleting the currently selected entry
		// this has to happen before we delete it in memory ofc
		if (UserEntrySelected()){

			if (m_activeLibEntry->runtime_ID == libEntry->runtime_ID) {

				SetDefaultMaterial();
			}		
		}

		// delete files
		fs::path libEntryFolderPath  = LibEntry_GetFolderPath(libEntry);
		try {
			fs::remove_all(libEntryFolderPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteMaterial: System error deleting directory \nError message: ", error.what());
			return;
		}
		
		// deleting from memory
		m_folderTree->DeleteLibEntry(libEntry->parent, positionInVector);


		// save
		SaveUserDirectoriesData();
	}

	void MaterialLibraryRegistry::LibEntry_Move(FolderNode* sourceNode, FolderNode* targetNode, systems::LibEntry* libEntry) {

		namespace fs = std::filesystem;

		// move material folder / copy dir and remove dir
		std::string entryName = libEntry->name; // temporary storing name here
		fs::path libraryDir = m_fileDirectories->GetLibraryDirectoryPath();

		// Copying and then removing works rn but is not very elegant. fs::rename() does not work and throws acces denied error
			//fs::rename(fromPath, toPath / fromPath.filename());

		fs::path fromPath = libraryDir / fs::path(sourceNode->pathFromRoot) / fs::path(entryName);
		fs::path toPath = libraryDir / fs::path(targetNode->pathFromRoot) / fs::path(entryName);
	
		// first copy files to new folder
		try { 
			fs::copy(fromPath, toPath , fs::copy_options::recursive);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveMaterial: System Error Copying directory: \nMessage: {}", error.what());
			return;
		}

		// then remove old folder
		try {
			fs::remove_all(fromPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::MoveMaterial: System Error Removing old directory: \nMessage: {}", error.what());
			return;
		}

		// unload thumbnail
		if (sourceNode == m_selectedFolderNode) {
			MnemosyEngine::GetInstance().GetThumbnailManager().RemoveLibEntryFromActiveThumbnails(libEntry);
		}

		// move in internal tree
		m_folderTree->MoveLibEntry(libEntry, sourceNode, targetNode);

		// make sure thumbnail gets loaded
		if (targetNode == m_selectedFolderNode) {
			MnemosyEngine::GetInstance().GetThumbnailManager().AddLibEntryToActiveThumbnails(targetNode->subEntries.back());
		}

		// save
		SaveUserDirectoriesData();
	}
	

	// TODO: handle skybox entry type
	void MaterialLibraryRegistry::LibEntry_Load(systems::LibEntry* libEntry) {

		double beginTime = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();

		ActiveLibEntry_SaveToFile();

		systems::LibEntryType type = libEntry->type;


		if (type == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {


			graphics::PbrMaterial* mat = LibProcedures::LibEntry_PbrMaterial_LoadFromFile_Multithreaded(libEntry,prettyPrintMaterialFiles);
			MNEMOSY_ASSERT(mat != nullptr, "This should not happen");

			MnemosyEngine::GetInstance().GetScene().SetPbrMaterial(mat);

			MNEMOSY_DEBUG("MaterialLibrary: Loaded PBR Material: {}", libEntry->name);
		}
		else if (type == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {

			graphics::UnlitMaterial* unlitMat = LibProcedures::LibEntry_UnlitMaterial_LoadFromFile(libEntry, prettyPrintMaterialFiles);

			MnemosyEngine::GetInstance().GetScene().SetUnlitMaterial(unlitMat);

			MNEMOSY_DEBUG("MaterialLibrary: Loaded Unlit Material: {}", libEntry->name);
		}
		else if (type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {
			MNEMOSY_DEBUG("MaterialLibrary: Loading Skyboxes is not yet supported: {}", libEntry->name);
		}


		// for the renderer we are storing the last selected entry type that is a mesh material (that means not a skybox) 
		// so that the renderer knows wich material type to use when rendering if a skybox entry type is currently selected

		if (m_activeLibEntry) {

			if (m_activeLibEntry->type != systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

				m_lastActiveMaterialLibEntry = m_activeLibEntry->type;
			}
		}
		else {
			m_lastActiveMaterialLibEntry = systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT;
		}

		m_activeLibEntry = libEntry;


		double endTime = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();
		MNEMOSY_TRACE("Loaded Material in {} seconds", endTime - beginTime);
	}

	void MaterialLibraryRegistry::ActiveLibEntry_SaveToFile() {

		if (!UserEntrySelected())
			return;

		fs::path thumbnailPath = fs::path(m_activeLibEntry->name + texture_fileSuffix_thumbnail);

		fs::path entryFolderPath = LibEntry_GetFolderPath(m_activeLibEntry);

		{ // Render thumbnail of active material
			fs::path thumbnailAbsolutePath = entryFolderPath / thumbnailPath;
			MnemosyEngine::GetInstance().GetThumbnailManager().RenderThumbnailForActiveLibEntry(m_activeLibEntry);
		}


		switch (m_activeLibEntry->type)
		{
		case mnemosy::systems::MNSY_ENTRY_TYPE_PBRMAT:

			LibProcedures::LibEntry_PbrMaterial_SaveToFile(m_activeLibEntry, &MnemosyEngine::GetInstance().GetScene().GetPbrMaterial(), prettyPrintMaterialFiles);
			break;
		case mnemosy::systems::MNSY_ENTRY_TYPE_UNLITMAT:

			LibProcedures::LibEntry_UnlitMaterial_SaveToFile(m_activeLibEntry, MnemosyEngine::GetInstance().GetScene().GetUnlitMaterial(),prettyPrintMaterialFiles);
			break;
		case mnemosy::systems::MNSY_ENTRY_TYPE_SKYBOX:
			break;

			LibProcedures::LibEntry_SkyboxMaterial_SaveToFile(m_activeLibEntry, &MnemosyEngine::GetInstance().GetScene().GetSkybox(),prettyPrintMaterialFiles);
		default:
			break;
		}

	}


	void MaterialLibraryRegistry::ActiveLibEntry_PbrMat_GenerateOpacityFromAlbedoAlpha(LibEntry* libEntry, graphics::PbrMaterial& activePbrMat) {

		namespace fs = std::filesystem;

		fs::path opacityMapPath = LibEntry_GetFolderPath(m_activeLibEntry) / fs::path(std::string(libEntry->name + texture_fileSuffix_opacity));

		//Generate opacity Texture
		MnemosyEngine::GetInstance().GetTextureGenerationManager().GenerateOpacityFromAlbedoAlpha(activePbrMat, opacityMapPath.generic_string().c_str(), true);

		// load the texture
		graphics::PictureError errorCheck;
		graphics::PictureInfo picInfo = graphics::Picture::ReadPicture(errorCheck, opacityMapPath.generic_string().c_str(),true, false,false);
		if (!errorCheck.wasSuccessfull) {

			MNEMOSY_ERROR("Failed to read opacity texture that was just generated. \nMessage: {}", errorCheck.what);

			// delete texture file we just created
			try {
				fs::remove_all(opacityMapPath);
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::GenerateOpacityFromAlbedoAlpha: System error deleting file \nError message: ", error.what());
				return;
			}

			return;
		}

		graphics::Texture* tex = new graphics::Texture();
		tex->GenerateOpenGlTexture(picInfo,true);

		if(picInfo.pixels)
			free(picInfo.pixels);

		activePbrMat.assignTexture(graphics::MNSY_TEXTURE_OPACITY, tex);

		ActiveLibEntry_SaveToFile();
	}

	void MaterialLibraryRegistry::ActiveLibEntry_PbrMat_GenerateChannelPackedTexture(LibEntry* libEntry, graphics::PbrMaterial& activeMat, std::string& suffix, graphics::ChannelPackType packType, graphics::ChannelPackComponent packComponent_R, graphics::ChannelPackComponent packComponent_G, graphics::ChannelPackComponent packComponent_B, graphics::ChannelPackComponent packComponent_A, unsigned int width, unsigned int height, uint8_t bitDepth) {

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


		std::string filename = libEntry->name + suffix + texture_fileExtentionTiff;
		std::filesystem::path channelPackedExportPath = LibEntry_GetFolderPath(m_activeLibEntry) / std::filesystem::path(filename);

		MnemosyEngine::GetInstance().GetTextureGenerationManager().GenerateChannelPackedTexture(activeMat, channelPackedExportPath.generic_string().c_str(),true,packType,packComponent_R,packComponent_G,packComponent_B,packComponent_A, width,height,bitDepth);

		// Enlist into suffixes of active mat
		activeMat.HasPackedTextures = true;
		activeMat.PackedTexturesSuffixes.push_back(suffix);

		// save to file
		fs::path folderPath = LibEntry_GetFolderPath(m_activeLibEntry);

		ActiveLibEntry_SaveToFile();

	}

	void MaterialLibraryRegistry::ActiveLibEntry_PbrMat_DeleteChannelPackedTexture(LibEntry* libEntry, graphics::PbrMaterial& activeMat, std::string suffix) {

		namespace fs = std::filesystem;

		MNEMOSY_ASSERT(activeMat.HasPackedTextures, "This function should not be called if the material does not contain any channel packed textures");
		MNEMOSY_ASSERT(!activeMat.PackedTexturesSuffixes.empty(), "This function should not be called if the material does not contain any channel packed textures, this assert should never happen because we already checked if the texture has packed textures, there must be a bug somewhere when loading or saving the material");

		// delete texture file

		std::string filename = libEntry->name + suffix + texture_fileExtentionTiff;
		fs::path filepath = LibEntry_GetFolderPath(m_activeLibEntry) / fs::path(filename);


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
		ActiveLibEntry_SaveToFile();

		MNEMOSY_INFO("Deleted channel packed texture {}", filepath.generic_string());

	}

	void MaterialLibraryRegistry::SetDefaultMaterial() {

		if (m_activeLibEntry) {

			if (m_activeLibEntry->type != systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {
				m_lastActiveMaterialLibEntry = m_activeLibEntry->type;
			}

		}

		m_activeLibEntry = nullptr;


		graphics::PbrMaterial* defaultMaterial = new graphics::PbrMaterial();
		MnemosyEngine::GetInstance().GetScene().SetPbrMaterial(defaultMaterial);
	}

	void MaterialLibraryRegistry::ActiveLibEntry_PbrMat_LoadTexture(graphics::PBRTextureType textureType, std::filesystem::path& filepath) {

		MNEMOSY_ASSERT(m_activeLibEntry != nullptr, "This should not happen");
		MNEMOSY_ASSERT(m_activeLibEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT, "It must be pbr if calling this method!");

		namespace fs = std::filesystem;

		graphics::PictureError err;
		

		bool convertGrayToRGB = textureType == graphics::PBRTextureType::MNSY_TEXTURE_ALBEDO || textureType == graphics::PBRTextureType::MNSY_TEXTURE_EMISSION;
		graphics::PictureInfo picInfo = graphics::Picture::ReadPicture(err, filepath.generic_string().c_str(),true, convertGrayToRGB, true);
		if (!err.wasSuccessfull) {
			MNEMOSY_ERROR("Unable to load Texture: {} \nMessage: {}",filepath.generic_string(),err.what);
			return;
		}

		graphics::Texture* tex = new graphics::Texture();
		tex->GenerateOpenGlTexture(picInfo,true);

		if (picInfo.pixels)
			free(picInfo.pixels);

		graphics::PbrMaterial& activeMat = MnemosyEngine::GetInstance().GetScene().GetPbrMaterial();
		systems::ExportManager& exportManager = MnemosyEngine::GetInstance().GetExportManager();

		fs::path materialDir = LibEntry_GetFolderPath(m_activeLibEntry);//  m_fileDirectories->GetLibraryDirectoryPath() / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name);

		std::string filename;
		fs::path exportPath;
		graphics::TextureFormat format = graphics::MNSY_RGB8;


		uint8_t numChannels, bitsPerChannel,bytesPerPixel;
		graphics::TexUtil::get_information_from_textureFormat(picInfo.textureFormat,numChannels,bitsPerChannel,bytesPerPixel);


		filename = graphics::TexUtil::get_filename_from_PBRTextureType(m_activeLibEntry->name,textureType);
		exportPath = materialDir / fs::path(filename);


		bool isGrayscaleImage = textureType != graphics::PBRTextureType::MNSY_TEXTURE_ALBEDO && textureType != graphics::PBRTextureType::MNSY_TEXTURE_NORMAL && textureType != graphics::PBRTextureType::MNSY_TEXTURE_EMISSION && textureType != graphics::PBRTextureType::MNSY_TEXTURE_NONE;

		// determine the output format. grayscale will be exported with only one channel, colored textures as RGB or RGBA depending on the input texture

		// Includes: Roughness, Metallic, AO, Height, Opacity
		if (isGrayscaleImage) {

			if (bitsPerChannel == 8) {
				format = graphics::MNSY_R8;
			}
			else if (bitsPerChannel == 16) {
				format = graphics::MNSY_R16;
			}
			else if (bitsPerChannel == 32) {
				format = graphics::MNSY_R32;
			}

			// special cases
			if (textureType == graphics::MNSY_TEXTURE_ROUGHNESS) {
				activeMat.IsSmoothnessTexture = false;
			}
		}
		
		// Includes: Albedo, Normal, Emission
		else {


			if (numChannels == 4) {
				if (bitsPerChannel == 8) {
					format = graphics::MNSY_RGBA8;
				}
				else if (bitsPerChannel == 16) {
					format = graphics::MNSY_RGBA16;
				}
				else if (bitsPerChannel == 32) {
					format = graphics::MNSY_RGBA32;
				}
			}
			else {
				if (bitsPerChannel == 8) {
					format = graphics::MNSY_RGB8;
				}
				else if (bitsPerChannel == 16) {
					format = graphics::MNSY_RGB16;
				}
				else if (bitsPerChannel == 32) {
					format = graphics::MNSY_RGB32;
				}
			}

			// special cases
			if (textureType == graphics::MNSY_TEXTURE_NORMAL) {
				activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGL);
			}
		}

		systems::TextureExportInfo exportInfo = systems::TextureExportInfo(exportPath,tex->GetWidth(), tex->GetHeight(), format, picInfo.isHalfFloat);
		exportManager.GLTextureExport(tex->GetID(),exportInfo);


		// update Material Meta data file.
		flcrm::JsonSettings matFile;
		bool success = false;

		fs::path dataFilePath = LibEntry_GetDataFilePath(m_activeLibEntry);

		matFile.FileOpen(success, dataFilePath, jsonKey_header, jsonMatKey_description);
		if(!success){
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadTextureForActiveMaterial: Error Opening Data File. Message: {}", matFile.ErrorStringLastGet());
		 	delete tex;
			return;
		}

		// set path to assigned in the data file
		{
			std::string jsonMatKey_path_ofTextureType = graphics::TexUtil::get_JsonMatKey_path_from_PBRTextureType(textureType);
			std::string jsonMatKey_assigned_ofTextureType = graphics::TexUtil::get_JsonMatKey_assigned_from_PBRTextureType(textureType);

			matFile.WriteBool(success,jsonMatKey_assigned_ofTextureType,true);
			matFile.WriteString(success,jsonMatKey_path_ofTextureType,filename);
		}


		matFile.FilePrettyPrintSet(prettyPrintMaterialFiles);

		matFile.FileClose(success, dataFilePath);

		// load texture to material
		activeMat.assignTexture(textureType, tex);

	}

	void MaterialLibraryRegistry::ActiveLibEntry_PbrMat_DeleteTexture(graphics::PBRTextureType textureType)
    {
		MNEMOSY_ASSERT(m_activeLibEntry != nullptr, "This should not happen");
		MNEMOSY_ASSERT(m_activeLibEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT, "It must be pbr if calling this method!");

  		namespace fs = std::filesystem;

		graphics::PbrMaterial& activeMat = MnemosyEngine::GetInstance().GetScene().GetPbrMaterial();

		if (!UserEntrySelected()) { // if its default material // should not be possible using the gui bc default material is not editable 
			activeMat.removeTexture(textureType);
			return;
		}


		bool success = false;

		fs::path dataFilePath = LibEntry_GetDataFilePath(m_activeLibEntry);

		flcrm::JsonSettings matFile;
		matFile.FileOpen(success, dataFilePath, jsonKey_header, jsonMatKey_description);


		fs::path materialDir = LibEntry_GetFolderPath(m_activeLibEntry); // m_fileDirectories->GetLibraryDirectoryPath() / fs::path(m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name);



		// find filename of the texture then remove it form disk and reset the path to "notAssigned" in the material meta data file.
		{

			std::string jsonMatKey_path_ofTextureType = graphics::TexUtil::get_JsonMatKey_path_from_PBRTextureType(textureType);


			fs::path textureFilePath = materialDir / fs::path(matFile.ReadString(success,jsonMatKey_path_ofTextureType,jsonKey_pathNotAssigned,false));
			try { fs::remove(textureFilePath); }
			catch (fs::filesystem_error e) { MNEMOSY_ERROR("MaterialLibraryRegistry::DeleteTextureOfActiveMaterial: System error deleting file.\nError message: {}", e.what()) }

			std::string jsonMatKey_assigned_ofTextureType = graphics::TexUtil::get_JsonMatKey_assigned_from_PBRTextureType(textureType);

			matFile.WriteBool(success,jsonMatKey_assigned_ofTextureType,false);
			matFile.WriteString(success,jsonMatKey_path_ofTextureType,jsonKey_pathNotAssigned);

			// special case texture types

			if (textureType == graphics::MNSY_TEXTURE_NORMAL) {

				activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGL);
				matFile.WriteInt(success,jsonMatKey_normalMapFormat,0);

			}
			else if(textureType == graphics::MNSY_TEXTURE_ROUGHNESS){
				
				activeMat.IsSmoothnessTexture = false;
				matFile.WriteBool(success,jsonMatKey_isSmoothness,false);
			}
			else if(textureType == graphics::MNSY_TEXTURE_EMISSION){
				activeMat.UseEmissiveAsMask = false;
				matFile.WriteBool(success,jsonMatKey_useEmissiveAsMask,false);
			}			
		}
		
		matFile.FilePrettyPrintSet(prettyPrintMaterialFiles);

		matFile.FileClose(success, dataFilePath);

		activeMat.removeTexture(textureType);
	}

	void MaterialLibraryRegistry::ActiveLibEntry_UnlitMat_LoadTexture(std::filesystem::path& filepath) {

		MNEMOSY_ASSERT(m_activeLibEntry != nullptr, "This should not happen");
		MNEMOSY_ASSERT(m_activeLibEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT, "It must be pbr if calling this method!");

		// load image texture to memory
		graphics::PictureError err;
		graphics::PictureInfo picInfo = graphics::Picture::ReadPicture(err, filepath.generic_string().c_str(), true, true ,true);
		if (!err.wasSuccessfull) {
			MNEMOSY_ERROR("Unable to load Texture: {} \nMessage: {}", filepath.generic_string(), err.what);
			return;
		}

		// create openGl texture
		graphics::Texture* tex = new graphics::Texture();
		tex->GenerateOpenGlTexture(picInfo, true);
		
		// free image data from main memory
		if (picInfo.pixels)
			free(picInfo.pixels);



		fs::path materialDir = LibEntry_GetFolderPath(m_activeLibEntry);
		std::string filename = std::string(m_activeLibEntry->name + texture_unlit_fileSuffix);			
		fs::path exportPath = materialDir / fs::path(filename);

		// export texture to entry folder
		systems::ExportManager& exportManager = MnemosyEngine::GetInstance().GetExportManager();
		systems::TextureExportInfo exportInfo = systems::TextureExportInfo(exportPath, tex->GetWidth(), tex->GetHeight(), picInfo.textureFormat, picInfo.isHalfFloat);
		exportManager.GLTextureExport(tex->GetID(), exportInfo);


		// update entry Meta data file.
		{
			fs::path dataFilePath = LibEntry_GetDataFilePath(m_activeLibEntry);

			flcrm::JsonSettings dataFile;
			bool success = false;

			dataFile.FileOpen(success, dataFilePath, jsonKey_header, jsonKey_unlit_description);
			if (!success) {
				MNEMOSY_ERROR("MaterialLibraryRegistry::LoadTextureForActiveUnlitMaterial: Error Opening Data File. Message: {}", dataFile.ErrorStringLastGet());
				delete tex;
				return;
			}


			dataFile.WriteBool(success, jsonKey_unlit_textureIsAssigned, true);
			dataFile.WriteString(success, jsonKey_unlit_texturePath, filename);


			dataFile.FilePrettyPrintSet(prettyPrintMaterialFiles);

			dataFile.FileClose(success, dataFilePath);
		}

		// assign texture to material		
		MnemosyEngine::GetInstance().GetScene().GetUnlitMaterial()->AssignTexture(tex);
	}

	void MaterialLibraryRegistry::ActiveLibEntry_UnlitMat_DeleteTexture() {
		MNEMOSY_ASSERT(m_activeLibEntry != nullptr, "This should not happen");
		MNEMOSY_ASSERT(m_activeLibEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT, "It must be UNLIT if calling this method!");


		namespace fs = std::filesystem;

		graphics::UnlitMaterial* unlitMat = MnemosyEngine::GetInstance().GetScene().GetUnlitMaterial();
		unlitMat->RemoveTexture();

		// remove file from disk
		{
			fs::path entryFolder = LibEntry_GetFolderPath(m_activeLibEntry);

			std::string filename = m_activeLibEntry->name + texture_unlit_fileSuffix;
			fs::path textureFilePath = entryFolder / fs::path(filename);

			if (fs::exists(textureFilePath)) {
				try { 
					fs::remove(textureFilePath); 
				}
				catch (fs::filesystem_error e) { 
					MNEMOSY_ERROR("System error deleting texture file.\nError message: {}", e.what()) 
				}
			}
		}

		// update data file
		{

			bool success = false;
			fs::path dataFilePath = LibEntry_GetDataFilePath(m_activeLibEntry);

			flcrm::JsonSettings dataFile;

			dataFile.FileOpen(success, dataFilePath, jsonKey_header, jsonKey_unlit_description);

			dataFile.WriteBool(success, jsonKey_unlit_textureIsAssigned, false);
			dataFile.WriteString(success, jsonKey_unlit_texturePath, jsonKey_pathNotAssigned);

			dataFile.FilePrettyPrintSet(prettyPrintMaterialFiles);

			dataFile.FileClose(success, dataFilePath);
		}


	}


	// TODO: implement
	void MaterialLibraryRegistry::ActiveLibEntry_Skybox_LoadTexture(std::filesystem::path& filepath)
	{
	}

	void MaterialLibraryRegistry::ActiveLibEntry_Skybox_DeleteTexture() {

		MNEMOSY_ASSERT(m_activeLibEntry != nullptr, "This should not happen");
		MNEMOSY_ASSERT(m_activeLibEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX, "It must be Skybox if calling this method!");

		namespace fs = std::filesystem;

		graphics::Skybox& skyboxMat = MnemosyEngine::GetInstance().GetScene().GetSkybox();
		skyboxMat.RemoveCubemap();

		// remove files from disk
		{
			fs::path entryFolder = LibEntry_GetFolderPath(m_activeLibEntry);

			// equirectangular file
			{
				fs::path equirectangularFile = entryFolder / fs::path(m_activeLibEntry->name + texture_skybox_fileSuffix_equirectangular);

				if (fs::exists(equirectangularFile)) {
					try {
						fs::remove(equirectangularFile);
					}
					catch (fs::filesystem_error e) {
						MNEMOSY_ERROR("System error deleting texture file.\nError message: {}", e.what())
					}
				}
			}

			// cubeColor file
			{
				fs::path cubeColorFile = entryFolder / fs::path(m_activeLibEntry->name + texture_skybox_fileSuffix_cubeColor);

				if (fs::exists(cubeColorFile)) {
					try {
						fs::remove(cubeColorFile);
					}
					catch (fs::filesystem_error e) {
						MNEMOSY_ERROR("System error deleting texture file.\nError message: {}", e.what())
					}
				}
			}

			// cubePrefilter file
			{
				fs::path cubePrefilterFile = entryFolder / fs::path(m_activeLibEntry->name + texture_skybox_fileSuffix_cubePrefilter);

				if (fs::exists(cubePrefilterFile)) {
					try {
						fs::remove(cubePrefilterFile);
					}
					catch (fs::filesystem_error e) {
						MNEMOSY_ERROR("System error deleting texture file.\nError message: {}", e.what())
					}
				}
			}

			// cubeIrradiance file
			{
				fs::path cubeIrradianceFile = entryFolder / fs::path(m_activeLibEntry->name + texture_skybox_fileSuffix_cubeIrradiance);

				if (fs::exists(cubeIrradianceFile)) {
					try {
						fs::remove(cubeIrradianceFile);
					}
					catch (fs::filesystem_error e) {
						MNEMOSY_ERROR("System error deleting texture file.\nError message: {}", e.what())
					}
				}
			}

		}

		// update data file
		{

			bool success = false;
			fs::path dataFilePath = LibEntry_GetDataFilePath(m_activeLibEntry);

			flcrm::JsonSettings dataFile;

			dataFile.FileOpen(success, dataFilePath, jsonKey_header, jsonKey_skybox_description);

			dataFile.WriteBool(success, jsonKey_skybox_textureIsAssigned, false);
			dataFile.WriteString(success, jsonKey_skybox_texturePath, jsonKey_pathNotAssigned);

			dataFile.FilePrettyPrintSet(prettyPrintMaterialFiles);

			dataFile.FileClose(success, dataFilePath);
		}
	}

	void MaterialLibraryRegistry::OpenFolderNode(FolderNode* node){


		ActiveLibEntry_SaveToFile();

		MNEMOSY_ASSERT(node != nullptr, "We cannot set a nullptr node as the selected node");

		m_selectedFolderNode = node;

		systems::ThumbnailManager& thumbnailManager = MnemosyEngine::GetInstance().GetThumbnailManager();

		// unload all currently loaded thumbnails
		thumbnailManager.UnloadAllThumbnails();

		if (m_selectedFolderNode->HasMaterials()) {

			for (int i = 0; i < m_selectedFolderNode->subEntries.size(); i++) {
				thumbnailManager.AddLibEntryToActiveThumbnails(m_selectedFolderNode->subEntries[i]);
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

	std::filesystem::path MaterialLibraryRegistry::GetLibraryPath()
	{
		return m_fileDirectories->GetLibraryDirectoryPath();
	}

	std::filesystem::path MaterialLibraryRegistry::Folder_GetFullPath(FolderNode* node)
	{
		return m_fileDirectories->GetLibraryDirectoryPath() / node->pathFromRoot;
	}

	std::filesystem::path MaterialLibraryRegistry::LibEntry_GetFolderPath(LibEntry* libEntry) {
		return LibProcedures::LibEntry_GetFolderPath(libEntry);
	}

	std::filesystem::path MaterialLibraryRegistry::LibEntry_GetDataFilePath(LibEntry* libEntry) {

		return LibProcedures::LibEntry_GetDataFilePath(libEntry);
	}

	// TODO handle Entry type skybox
	std::vector<std::string> MaterialLibraryRegistry::ActiveLibEntry_GetTexturePaths() {

		namespace fs = std::filesystem;

		std::vector<std::string> paths;

		if (!UserEntrySelected()) {
			MNEMOSY_ERROR("MaterialLibraryRegistry::GetFilepathsOfActiveMat: No active material");
			return paths;
		}

		//fs::path libDir = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();
		fs::path entryFolder = LibEntry_GetFolderPath(m_activeLibEntry);

		std::string entryName = m_activeLibEntry->name;


		if (m_activeLibEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {

			graphics::PbrMaterial& activeMat = MnemosyEngine::GetInstance().GetScene().GetPbrMaterial();

			// TODO: could rewrite this as a loop

			// include each txture that is assigned to this material
			if (activeMat.isAlbedoAssigned()) {
				fs::path p = entryFolder / fs::path(entryName + texture_fileSuffix_albedo);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isNormalAssigned()) {
				fs::path p = entryFolder / fs::path(entryName + texture_fileSuffix_normal);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isRoughnessAssigned()) {
				fs::path p = entryFolder / fs::path(entryName + texture_fileSuffix_roughness);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isMetallicAssigned()) {
				fs::path p = entryFolder / fs::path(entryName + texture_fileSuffix_metallic);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isAoAssigned()) {
				fs::path p = entryFolder / fs::path(entryName + texture_fileSuffix_ambientOcclusion);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isEmissiveAssigned()) {
				fs::path p = entryFolder / fs::path(entryName + texture_fileSuffix_emissive);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isHeightAssigned()) {
				fs::path p = entryFolder / fs::path(entryName + texture_fileSuffix_height);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isOpacityAssigned()) {
				fs::path p = entryFolder / fs::path(entryName + texture_fileSuffix_opacity);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}


			// include channel packed textures as well
			if (activeMat.HasPackedTextures) {

				if (!activeMat.PackedTexturesSuffixes.empty()) {

					for (int i = 0; i < activeMat.PackedTexturesSuffixes.size(); i++) {

						std::string filename = entryName + activeMat.PackedTexturesSuffixes[i] + texture_fileExtentionTiff;
						fs::path p = entryFolder / fs::path(filename);

						if (fs::exists(p)) {
							paths.push_back(p.generic_string());
						}
					}

				}
			}
		}
		// = Unlit Material
		else if (m_activeLibEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {

			graphics::UnlitMaterial* mat = MnemosyEngine::GetInstance().GetScene().GetUnlitMaterial();

			if (mat->TextureIsAssigned()) {

				std::string filename = m_activeLibEntry->name + texture_unlit_fileSuffix;
				fs::path p = entryFolder / fs::path(filename);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
		}
		// = Skybox Material
		else if (m_activeLibEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

			graphics::Skybox& skyboxMat = MnemosyEngine::GetInstance().GetScene().GetSkybox();

			if (skyboxMat.IsTextureAssigned()) {


				std::string filename = m_activeLibEntry->name + texture_skybox_fileSuffix_equirectangular;

				fs::path p = entryFolder / fs::path(filename);
				
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
		}

		return paths;
	}

	bool MaterialLibraryRegistry::SearchLibEntriesForKeyword(const std::string& keyword) {

		return m_folderTree->CollectMaterialsFromSearchKeyword(keyword);
	}

	std::vector<systems::LibEntry*>& MaterialLibraryRegistry::GetSearchResultsList() {

		return m_folderTree->GetSearchResultsList();
	}

	systems::LibEntryType MaterialLibraryRegistry::GetEntryTypeToRenderWith() {

		if (m_activeLibEntry) {

			if (m_activeLibEntry->type != systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {
				return m_activeLibEntry->type;
			}
			else {
				return m_lastActiveMaterialLibEntry;
			}
		}
		
		return systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT;
	}

	bool MaterialLibraryRegistry::IsActiveEntry(uint16_t runtimeID) {
		if (UserEntrySelected()) {
			if (m_activeLibEntry->runtime_ID == runtimeID) {
				return true;
			}
		}

		return false;
	}




} // !mnemosy::systems
