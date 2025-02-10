#include "Include/Systems/MaterialLibraryRegistry.h"

#include "Include/MnemosyEngine.h"
#include "Include/MnemosyConfig.h"
#include "Include/Core/Log.h"
#include "Include/Core/Clock.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Core/Utils/StringUtils.h"
#include <json.hpp>

#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/TextureDefinitions.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Systems/SkyboxAssetRegistry.h"
#include "Include/Graphics/Cubemap.h"
#include "Include/Graphics/Utils/Picture.h"
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

#include <FulcrumUtils/Flcrm_Log.hpp>


namespace mnemosy::systems {
	// == public methods

	void MaterialLibraryRegistry::Init()
	{
		namespace fs = std::filesystem;


		m_folderTree = nullptr;
		inSearchMode = false;

		m_fileDirectories = nullptr;
		m_selectedFolderNode = nullptr;


		// data file 
		prettyPrintDataFile = false;
		prettyPrintMaterialFiles = true;

		m_fileDirectories = &MnemosyEngine::GetInstance().GetFileDirectories();

		m_lastActiveMaterialLibEntry = systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT;

		// must load lib collection before loading a specific data file.
		LibCollections_LoadFromFile();


		if (m_libCollection_currentSlected_id != -1) {

			LibCollections_SwitchActiveCollection(m_libCollection_currentSlected_id);
		}
	}

	void MaterialLibraryRegistry::Shutdown() {

		ActiveLibCollection_SaveToFile();

		if (m_folderTree) {
			m_folderTree->Shutdown();
			delete m_folderTree;
		}
	}


	const bool MaterialLibraryRegistry::LibCollection_LoadIntoActiveTree(std::filesystem::path& folderPath) {


		namespace fs = std::filesystem;


		//fs::path folderPathWide = core::StringUtils::u8path_from_path(folderPath);

		if (!fs::exists(folderPath)) {
			return false;
		}

		fs::path dataFilePath = folderPath / fs::path("MnemosyMaterialLibraryData.mnsydata");

		if (!LibProcedures::CheckDataFile(dataFilePath)) {

			MNEMOSY_ERROR("Data file is invalid or not existing: {}", dataFilePath.generic_string());
			return false;
		}		


		std::ifstream dataFileStream;
		dataFileStream.open(dataFilePath);

		nlohmann::json readFile;
		try {
			readFile = nlohmann::json::parse(dataFileStream);
		} catch (nlohmann::json::parse_error err) {
			MNEMOSY_ERROR("Error Parsing File. Message: {}", err.what());
			dataFileStream.close();

			// clear folder tree if neccesary
			if (m_folderTree) {
				m_folderTree->Shutdown();
				delete m_folderTree;
				m_folderTree = nullptr;
			}


			return false;
		}
		dataFileStream.close();


		// clear folder tree if neccesary
		if (m_folderTree) {
			m_folderTree->Shutdown();
			delete m_folderTree;
			m_folderTree = nullptr;
		}

		m_folderTree = new FolderTree();

		m_folderTree->Init();
		m_folderTree->LoadFromJson(readFile);

		return true;

	}

  	void MaterialLibraryRegistry::ActiveLibCollection_SaveToFile() {
		namespace fs = std::filesystem;


		if (!LibCollections_IsAnyActive()) {
			return;
		}

		fs::path dataFile = ActiveLibCollection_GetDataFilePath();

		LibProcedures::CheckDataFile(dataFile);

		nlohmann::json* LibraryDirectoriesJson = m_folderTree->WriteToJson();

		std::ofstream dataFileStream;
		dataFileStream.open(dataFile);


		bool savingSuccess = true;

		try {

			dataFileStream << LibraryDirectoriesJson->dump(-1);
			//if (prettyPrintDataFile) {
			//	dataFileStream << LibraryDirectoriesJson->dump(4);
			//}
			//else {
			//}

		}
		catch (nlohmann::json::type_error jErr) {

			MNEMOSY_ERROR("Failed to save Library data file correctly. Message: {}", jErr.what());
			savingSuccess = false;
		}
		
		
		dataFileStream.close();

		delete LibraryDirectoriesJson;

		if (savingSuccess) {

			// copy and make a backup file.			
			fs::path copyLocation = dataFile.parent_path() / fs::path("MnemosyMaterialLibraryData_BACKUP_COPY.mnsydata");
			fs::copy_file(dataFile, copyLocation, fs::copy_options::overwrite_existing);
		}
  	}

	void MaterialLibraryRegistry::SaveCurrentSate() {

		ActiveLibEntry_SaveToFile();
		ActiveLibCollection_SaveToFile();
		LibCollections_SaveToFile();
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

		MNEMOSY_ASSERT(LibCollections_IsAnyActive(), "One must be selected to be able to calls this method");

		if (node->IsRoot()) {
			MNEMOSY_WARN("You can't change the name of the root directory");
			return;
		}

		// store old path because pathFromRoot is upadeted inside RenameFolder() method
		fs::path libraryDir = ActiveLibCollection_GetFolderPath();


		fs::path oldPath = libraryDir / node->GetPathFromRoot();// fs::path(node->pathFromRoot);
		std::string oldName = node->name;

		// rename internally
		m_folderTree->RenameFolder(node, newName);

		// rename files on disk
		fs::path newPath =  libraryDir / node->GetPathFromRoot();

		//newPath = fs::u8path(newPath.generic_string());

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

		fs::path libraryDir = ActiveLibCollection_GetFolderPath();
		fs::path fromPath = libraryDir / dragSource->GetPathFromRoot();
		fs::path toPath = libraryDir / dragTarget->GetPathFromRoot();


		// Copying and then removing works rn but is not very elegant. fs::rename() does not work and throws acces denied error
			//fs::rename(fromPath, toPath / fromPath.filename());

		try { // copy directory to new location
			fs::copy(fromPath, toPath / fromPath.filename(), fs::copy_options::recursive);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("System Error Copying directory: \nMessage: {}", error.what());
			return;
		}

		try { // remove old directory
			fs::remove_all(fromPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("System Error Removing old directory: \nMessage: {}", error.what());

			// if this happens we should clean up the copy we just did
			try {
				fs::remove_all(toPath / fromPath.filename());
			}
			catch (fs::filesystem_error error2) {
				MNEMOSY_ERROR("System Error Removing directory that was just copied, if this happend something went really wrong! \nMessage: {}", error.what());
			}
			return;
		}

		// Updating Internal Data
		m_folderTree->MoveFolder(dragSource, dragTarget);


		// should prob save to data file here
		ActiveLibCollection_SaveToFile();
	}

	void MaterialLibraryRegistry::DeleteAndKeepChildren(FolderNode* node) {

		// never delete root node
		if (node->IsRoot()) {
			MNEMOSY_WARN("You can't delete the root directory");
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
				MNEMOSY_ERROR("System error deleting directory: {} \nError Message: {} ", folderPath.generic_string(), error.what());
			}
		}

		m_folderTree->DeleteFolderHierarchy(node);
	}
		
	void MaterialLibraryRegistry::LibEntry_CreateNew(FolderNode* node, LibEntryType type , std::string& name) {

		namespace fs = std::filesystem;

		LibEntry* libEntry = m_folderTree->CreateNewLibEntry(node, type, name);

		// create directory for entry
		fs::path entryDirectory = LibEntry_GetFolderPath(libEntry);

		try {
			fs::create_directory(entryDirectory);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("System error creating directory. \nError message: {}", error.what());

			int posInVector = -1;
			for (unsigned int i = 0; i < node->subEntries.size(); i++) {

				if (node->subEntries[i]->runtime_ID == libEntry->runtime_ID) {
					posInVector = i;
				}
			}
			MNEMOSY_ASSERT(posInVector != -1, "This should not happen because we just added the material");


			m_folderTree->DeleteLibEntry(node, posInVector);

			return;
		}


		fs::path pathToEntryThumbnail = entryDirectory / fs::path(libEntry->name + texture_fileSuffix_thumbnail);
		
		if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {


			// Copy Default thumbnail image and if it doesn't exitst render from scratch
			fs::path pathToDefaultThumbnail = m_fileDirectories->GetTexturesPath() / fs::path("default_thumbnail_pbr.ktx2");

			if (fs::exists(pathToDefaultThumbnail)) {

				try {
					fs::copy_file(pathToDefaultThumbnail, pathToEntryThumbnail);
				}
				catch (fs::filesystem_error error) {
					MNEMOSY_ERROR("System error copying file. \nError message: {}", error.what());					
					MnemosyEngine::GetInstance().GetThumbnailManager().RenderThumbnailForAnyLibEntry_Slow_Fallback(libEntry);
				}
			}
			else {
				MnemosyEngine::GetInstance().GetThumbnailManager().RenderThumbnailForAnyLibEntry_Slow_Fallback(libEntry);
			}

			// create default material data file
			LibProcedures::LibEntry_PbrMaterial_CreateNewDataFile(libEntry,prettyPrintMaterialFiles);
		}
		else if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {

			// Copy Default thumbnail image and if it doesn't exitst render from scratch
			fs::path pathToDefaultThumbnail = m_fileDirectories->GetTexturesPath() / fs::path("default_thumbnail_unlit.ktx2");

			if (fs::exists(pathToDefaultThumbnail)) {

				try {
					fs::copy_file(pathToDefaultThumbnail, pathToEntryThumbnail);
				}
				catch (fs::filesystem_error error) {
					MNEMOSY_ERROR("System error copying file. \nError message: {}", error.what());
					MnemosyEngine::GetInstance().GetThumbnailManager().RenderThumbnailForAnyLibEntry_Slow_Fallback(libEntry);
				}
			}
			else {
				MnemosyEngine::GetInstance().GetThumbnailManager().RenderThumbnailForAnyLibEntry_Slow_Fallback(libEntry);
			}

			// create default material data file
			LibProcedures::LibEntry_UnlitMaterial_CreateNewDataFile(libEntry,prettyPrintMaterialFiles);
		}
		else if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

			// Copy Default thumbnail image and if it doesn't exitst render from scratch
			fs::path pathToDefaultThumbnail = m_fileDirectories->GetTexturesPath() / fs::path("default_thumbnail_skybox.ktx2");

			if (fs::exists(pathToDefaultThumbnail)) {

				try {
					fs::copy_file(pathToDefaultThumbnail, pathToEntryThumbnail);
				}
				catch (fs::filesystem_error error) {
					MNEMOSY_ERROR("System error copying file. \nError message: {}", error.what());
					MnemosyEngine::GetInstance().GetThumbnailManager().RenderThumbnailForAnyLibEntry_Slow_Fallback(libEntry);
				}
			}
			else {
				MnemosyEngine::GetInstance().GetThumbnailManager().RenderThumbnailForAnyLibEntry_Slow_Fallback(libEntry);
			}

			// create default material data file
			LibProcedures::LibEntry_SkyboxMaterial_CreateNewDataFile(libEntry, prettyPrintMaterialFiles);
		}

		// check if it was created in the currently opend folder
		if (node == m_selectedFolderNode) {
			// make sure it gets loaded

			MnemosyEngine::GetInstance().GetThumbnailManager().AddLibEntryToActiveThumbnails(libEntry);
		}


		ActiveLibCollection_SaveToFile();


		LibEntry_Load(libEntry);
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
		ActiveLibCollection_SaveToFile();
	}
		
	void MaterialLibraryRegistry::LibEntry_Delete(systems::LibEntry* libEntry, unsigned int positionInVector) {

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
			MNEMOSY_ERROR("System error deleting directory \nError message: ", error.what());
			return;
		}
		
		// deleting from memory
		m_folderTree->DeleteLibEntry(libEntry->parent, positionInVector);


		// save
		ActiveLibCollection_SaveToFile();
	}

	void MaterialLibraryRegistry::LibEntry_Move(FolderNode* sourceNode, FolderNode* targetNode, systems::LibEntry* libEntry) {

		namespace fs = std::filesystem;

		// move material folder / copy dir and remove dir
		std::string entryName = libEntry->name; // temporary storing name here
		fs::path libraryDir = ActiveLibCollection_GetFolderPath();

		// Copying and then removing works rn but is not very elegant. fs::rename() does not work and throws acces denied error
			//fs::rename(fromPath, toPath / fromPath.filename());

		fs::path fromPath = libraryDir / sourceNode->GetPathFromRoot() / fs::u8path(entryName);//  fs::path(sourceNode->pathFromRoot) / fs::path(entryName);
		fs::path toPath   = libraryDir / targetNode->GetPathFromRoot() / fs::u8path(entryName); // fs::path(targetNode->pathFromRoot) / fs::path(entryName);
	
		// first copy files to new folder
		try { 
			fs::copy(fromPath, toPath , fs::copy_options::recursive);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("System Error Copying directory: \nMessage: {}", error.what());
			return;
		}

		// then remove old folder
		try {
			fs::remove_all(fromPath);
		}
		catch (fs::filesystem_error error) {
			MNEMOSY_ERROR("System Error Removing old directory: \nMessage: {}", error.what());
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
		ActiveLibCollection_SaveToFile();
	}
	
	void MaterialLibraryRegistry::LibEntry_Load(systems::LibEntry* libEntry) {

		double beginTime = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();

		ActiveLibEntry_SaveToFile(); // save current selected entry first before switching

		systems::LibEntryType type = libEntry->type;


		if (type == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {

			

			graphics::PbrMaterial* mat = LibProcedures::LibEntry_PbrMaterial_LoadFromFile_Multithreaded(libEntry,prettyPrintMaterialFiles);
			MNEMOSY_ASSERT(mat != nullptr, "This should not happen");

			MnemosyEngine::GetInstance().GetScene().SetPbrMaterial(mat);

			MNEMOSY_DEBUG("Loaded PBR Material: {}", libEntry->name);
		}
		else if (type == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {

			graphics::UnlitMaterial* unlitMat = LibProcedures::LibEntry_UnlitMaterial_LoadFromFile(libEntry, prettyPrintMaterialFiles);

			MnemosyEngine::GetInstance().GetScene().SetUnlitMaterial(unlitMat);

			MNEMOSY_DEBUG("Loaded Unlit Material: {}", libEntry->name);
		}
		else if (type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

			fs::path p = LibEntry_GetFolderPath(libEntry);
			
			graphics::Skybox* sky = LibProcedures::LibEntry_SkyboxMaterial_LoadFromFile(p,libEntry->name,prettyPrintMaterialFiles);

			graphics::Scene& scene = MnemosyEngine::GetInstance().GetScene();
			
			scene.SetSkybox(sky);

			MnemosyEngine::GetInstance().GetRenderer().SetShaderSkyboxUniforms(scene.userSceneSettings,*sky);

			MNEMOSY_DEBUG("Loaded Skybox Material: {}", libEntry->name);
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

		if (!LibCollections_IsAnyActive()) {
			return;
		}


		if (!UserEntrySelected())
			return;

		//fs::path thumbnailPath = fs::u8path(m_activeLibEntry->name + texture_fileSuffix_thumbnail);

		fs::path entryFolderPath = LibEntry_GetFolderPath(m_activeLibEntry);

		{ // Render thumbnail of active material
			//fs::path thumbnailAbsolutePath = entryFolderPath / thumbnailPath;
			MnemosyEngine::GetInstance().GetThumbnailManager().RenderThumbnailForActiveLibEntry(m_activeLibEntry);
		}


		switch (m_activeLibEntry->type)
		{
		case systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT:
			LibProcedures::LibEntry_PbrMaterial_SaveToFile(m_activeLibEntry, &MnemosyEngine::GetInstance().GetScene().GetPbrMaterial(), prettyPrintMaterialFiles);
			break;
		case systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT:

			LibProcedures::LibEntry_UnlitMaterial_SaveToFile(m_activeLibEntry, MnemosyEngine::GetInstance().GetScene().GetUnlitMaterial(),prettyPrintMaterialFiles);
			break;
		case systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX:
						
			LibProcedures::LibEntry_SkyboxMaterial_SaveToFile(m_activeLibEntry, &MnemosyEngine::GetInstance().GetScene().GetSkybox(),prettyPrintMaterialFiles);
			break;
		default:
			return;
		}

	}

	void MaterialLibraryRegistry::ActiveLibEntry_PbrMat_GenerateOpacityFromAlbedoAlpha(LibEntry* libEntry, graphics::PbrMaterial& activePbrMat) {

		namespace fs = std::filesystem;

		fs::path opacityMapPath = LibEntry_GetFolderPath(m_activeLibEntry) / fs::u8path(libEntry->name + texture_fileSuffix_opacity);

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
				MNEMOSY_ERROR("System error deleting file \nError message: ", error.what());
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
		std::filesystem::path channelPackedExportPath = LibEntry_GetFolderPath(m_activeLibEntry) / std::filesystem::u8path(filename);

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
			MNEMOSY_ERROR("System error deleting file \nError message: ", error.what());
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

		ActiveLibEntry_SaveToFile();

		if (m_activeLibEntry) {

			if (m_activeLibEntry->type != systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {
				m_lastActiveMaterialLibEntry = m_activeLibEntry->type;
			}

		}

		m_activeLibEntry = nullptr;


		graphics::PbrMaterial* defaultMaterial = new graphics::PbrMaterial();
		MnemosyEngine::GetInstance().GetScene().SetPbrMaterial(defaultMaterial);
	}

	bool MaterialLibraryRegistry::UserEntrySelected()
	{

		if (!LibCollections_IsAnyActive()) {
			return false;
		}

		if (!m_activeLibEntry) {
			return false;
		}

		return true;
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

		graphics::TextureFormat format = graphics::MNSY_RGB8;


		uint8_t numChannels, bitsPerChannel,bytesPerPixel;
		graphics::TexUtil::get_information_from_textureFormat(picInfo.textureFormat,numChannels,bitsPerChannel,bytesPerPixel);


		std::string filename = graphics::TexUtil::get_filename_from_PBRTextureType(m_activeLibEntry->name,textureType);
		fs::path exportPath  = materialDir / fs::u8path(filename);


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
			MNEMOSY_ERROR("Error Opening Data File. Message: {}", matFile.ErrorStringLastGet());
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
			// this is the key too lookup the path not the path itself
			std::string jsonMatKey_path_ofTextureType = graphics::TexUtil::get_JsonMatKey_path_from_PBRTextureType(textureType);

			std::string textureFileName = matFile.ReadString(success, jsonMatKey_path_ofTextureType, jsonKey_pathNotAssigned, false);
			fs::path textureFilePath = materialDir / fs::u8path(textureFileName);
			try { 
				fs::remove(textureFilePath); 
			}
			catch (fs::filesystem_error e) { 
				MNEMOSY_ERROR("System error deleting file.\nError message: {}", e.what()); 
			}

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
		fs::path exportPath = materialDir / fs::u8path(filename);

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
				MNEMOSY_ERROR("Error Opening Data File. Message: {}", dataFile.ErrorStringLastGet());
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

		// calculate uv tiling automatically based on width & height
		{
			// calculate uv tiling based on texture witdth & height

			float x = (float)tex->GetWidth();  // e.g 1024
			float y = (float)tex->GetHeight(); // e.g 512

			float aspect_y = x / y; // e.g. 1024 /  512 = 2
			float aspect_x = y / x; // e.g. 512  / 1024 = 0.5

			float uv_tile_x = 1.0f;
			float uv_tile_y = 1.0f;

			if (aspect_y > 1.00000000f) {
				uv_tile_y = aspect_y;
			}
			if (aspect_x > 1.00000000f) {
				uv_tile_x = aspect_x;
			}

			MnemosyEngine::GetInstance().GetScene().GetUnlitMaterial()->UVTilingX = uv_tile_x;
			MnemosyEngine::GetInstance().GetScene().GetUnlitMaterial()->UVTilingY = uv_tile_y;
		}


		

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
			fs::path textureFilePath = entryFolder / fs::u8path(filename);

			if (fs::exists(textureFilePath)) {
				try { 
					fs::remove(textureFilePath); 
				}
				catch (fs::filesystem_error e) { 
					MNEMOSY_ERROR("System error deleting texture file.\nError message: {}", e.what());
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

	void MaterialLibraryRegistry::ActiveLibEntry_Skybox_LoadTexture(std::filesystem::path& filepath)
	{

		//double start = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();

		MNEMOSY_ASSERT(m_activeLibEntry != nullptr, "This should not happen");
		MNEMOSY_ASSERT(m_activeLibEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX, "It must be pbr if calling this method!");

		// first load equirectangular from file.
		graphics::PictureError err;
		graphics::PictureInfo picInfo = graphics::Picture::ReadPicture(err, filepath.generic_string().c_str(), true, true, false);
		if (!err.wasSuccessfull) {

			MNEMOSY_ERROR("Unable to load Texture: {} \nMessage: {}", filepath.generic_string(), err.what);			
			return;
		}

		if (picInfo.width >= 8 * 1024 + 512) { // over 8k textures are not supported atm because file sizes with RGB 32 bit get super big and libKtx has some limit on large file sizes.

			if (picInfo.pixels)
				free(picInfo.pixels);

			MNEMOSY_ERROR("Too large image sizes are not supported for skyboxes because they are stored in high bit depth (32bit) format and loading becomes unstable.  You may try storing them as non skybox unlit texture materials instead.");
			return;
		}

		graphics::Texture* equirectangularTex = new graphics::Texture();

		equirectangularTex->GenerateOpenGlTexture(picInfo,true);

		if (picInfo.pixels) {
			free(picInfo.pixels);
		}

		std::string entryName = m_activeLibEntry->name;
		fs::path entryFolder = LibEntry_GetFolderPath(m_activeLibEntry);

		// export or copy equirectangular to new location as hdr
		{
			fs::path equirectangularFilePath = entryFolder / fs::u8path(entryName + texture_skybox_fileSuffix_equirectangular);

			//if its an hdr we are loading we might aswell just copy the file, this is a common case and improves performance significantly for bigger files
			graphics::ImageFileFormat fileFormat = graphics::TexUtil::get_imageFileFormat_from_fileExtentionString(filepath.extension().generic_string());

			if (fileFormat == graphics::ImageFileFormat::MNSY_FILE_FORMAT_HDR) {
				try {
					fs::copy_file(filepath, equirectangularFilePath,fs::copy_options::overwrite_existing);
				}
				catch (fs::filesystem_error e) {
					MNEMOSY_WARN("System error copying file. \nMessage {}", e.what());

					// if copy fails we fallback to generating it manually
					systems::ExportManager& exportManager = MnemosyEngine::GetInstance().GetExportManager();

					systems::TextureExportInfo exportInfo = systems::TextureExportInfo(equirectangularFilePath, equirectangularTex->GetWidth(), equirectangularTex->GetHeight(), picInfo.textureFormat, picInfo.isHalfFloat, false);

					exportManager.GLTextureExport(equirectangularTex->GetID(), exportInfo);
				}
			}
			else { // otherwise export it from the gl texture
				systems::ExportManager& exportManager = MnemosyEngine::GetInstance().GetExportManager();
				systems::TextureExportInfo exportInfo = systems::TextureExportInfo(equirectangularFilePath, equirectangularTex->GetWidth(), equirectangularTex->GetHeight(), picInfo.textureFormat, picInfo.isHalfFloat, false);
				exportManager.GLTextureExport(equirectangularTex->GetID(), exportInfo);
			}
		}

		graphics::Skybox& skybox = MnemosyEngine::GetInstance().GetScene().GetSkybox();

		// generate cubemaps , assign them to skybox and export them as ktx2 files 
		{
			// cube irradiance
			{
				fs::path cubeIrradiancePath = entryFolder / fs::u8path(entryName + texture_skybox_fileSuffix_cubeIrradiance);

				graphics::Cubemap* irradCube = new graphics::Cubemap();
				irradCube->GenerateOpenGlCubemap_FromEquirecangularTexture(*equirectangularTex, graphics::CubemapType::MNSY_CUBEMAP_TYPE_IRRADIANCE,false,0);

				graphics::KtxImage ktx;
				ktx.SaveCubemap(cubeIrradiancePath.generic_string().c_str(), irradCube->GetGlID(), equirectangularTex->GetTextureFormat(), irradCube->GetResolution(),false);

				skybox.AssignCubemap(irradCube, graphics::CubemapType::MNSY_CUBEMAP_TYPE_IRRADIANCE);

			}

			// cube prefilter
			{
				fs::path cubePrefilterPath = entryFolder / fs::u8path(entryName + texture_skybox_fileSuffix_cubePrefilter);

				graphics::Cubemap* cube = new graphics::Cubemap();
				cube->GenerateOpenGlCubemap_FromEquirecangularTexture(*equirectangularTex, graphics::CubemapType::MNSY_CUBEMAP_TYPE_PREFILTER, false, 0);

				graphics::KtxImage ktx;
				ktx.SaveCubemap(cubePrefilterPath.generic_string().c_str(), cube->GetGlID(), equirectangularTex->GetTextureFormat(), cube->GetResolution(), true);

				skybox.AssignCubemap(cube, graphics::CubemapType::MNSY_CUBEMAP_TYPE_PREFILTER);
			}

		}

		// update data file
		{
			fs::path dataFilePath = LibEntry_GetDataFilePath(m_activeLibEntry);

			flcrm::JsonSettings dataFile;

			bool success = false;

			dataFile.FileOpen(success,dataFilePath,jsonKey_header,jsonKey_skybox_description);
			if(!success) {
				MNEMOSY_WARN("Error opening entry data file. Message: {}", dataFile.ErrorStringLastGet());
			}

			dataFile.WriteBool(success, jsonKey_skybox_textureIsAssigned, true);
			std::string equirectFilename = entryName + texture_skybox_fileSuffix_equirectangular;

			dataFile.WriteString(success, jsonKey_skybox_texturePath, equirectFilename);

			dataFile.FilePrettyPrintSet(prettyPrintMaterialFiles);

			dataFile.FileClose(success, dataFilePath);
		}


		//double end = MnemosyEngine::GetInstance().GetClock().GetTimeSinceLaunch();

		//MNEMOSY_TRACE("Loaded and gen skybox: {} sec", end - start);

	}

	void MaterialLibraryRegistry::ActiveLibEntry_Skybox_DeleteTexture() {

		MNEMOSY_ASSERT(m_activeLibEntry != nullptr, "This should not happen");
		MNEMOSY_ASSERT(m_activeLibEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX, "It must be Skybox if calling this method!");

		namespace fs = std::filesystem;

		graphics::Skybox& skyboxMat = MnemosyEngine::GetInstance().GetScene().GetSkybox();
		skyboxMat.RemoveAllCubemaps();

		// remove files from disk
		{
			fs::path entryFolder = LibEntry_GetFolderPath(m_activeLibEntry);

			// equirectangular file
			{
				fs::path equirectangularFile = entryFolder / fs::u8path(m_activeLibEntry->name + texture_skybox_fileSuffix_equirectangular);

				if (fs::exists(equirectangularFile)) {
					try {
						fs::remove(equirectangularFile);
					}
					catch (fs::filesystem_error e) {
						MNEMOSY_ERROR("System error deleting texture file.\nError message: {}", e.what());
					}
				}
			}

			// cubePrefilter file
			{
				fs::path cubePrefilterFile = entryFolder / fs::u8path(m_activeLibEntry->name + texture_skybox_fileSuffix_cubePrefilter);

				if (fs::exists(cubePrefilterFile)) {
					try {
						fs::remove(cubePrefilterFile);
					}
					catch (fs::filesystem_error e) {
						MNEMOSY_ERROR("System error deleting texture file.\nError message: {}", e.what());
					}
				}
			}

			// cubeIrradiance file
			{
				fs::path cubeIrradianceFile = entryFolder / fs::u8path(m_activeLibEntry->name + texture_skybox_fileSuffix_cubeIrradiance);

				if (fs::exists(cubeIrradianceFile)) {
					try {
						fs::remove(cubeIrradianceFile);
					}
					catch (fs::filesystem_error e) {
						MNEMOSY_ERROR("System error deleting texture file.\nError message: {}", e.what());
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

	// ======== Getters ========

	FolderNode* MaterialLibraryRegistry::GetRootFolder() {

		MNEMOSY_ASSERT(m_folderTree != nullptr, "This should not be called if no collection is active");

		return m_folderTree->GetRootPtr();
	}

	FolderNode* MaterialLibraryRegistry::GetFolderByID(FolderNode* node, const unsigned int id) {

		MNEMOSY_ASSERT(m_folderTree != nullptr, "This should not be called if no collection is active");
		return m_folderTree->RecursivGetNodeByID(node,id);
	}

	std::filesystem::path MaterialLibraryRegistry::Folder_GetFullPath(FolderNode* node)
	{
		return ActiveLibCollection_GetFolderPath() /  node->GetPathFromRoot();


		//return ActiveLibCollection_GetFolderPath() / node->pathFromRoot;
	}

	std::filesystem::path MaterialLibraryRegistry::LibEntry_GetFolderPath(LibEntry* libEntry) {
		return LibProcedures::LibEntry_GetFolderPath(libEntry);
	}

	std::filesystem::path MaterialLibraryRegistry::LibEntry_GetDataFilePath(LibEntry* libEntry) {

		return LibProcedures::LibEntry_GetDataFilePath(libEntry);
	}

	
	std::vector<std::string> MaterialLibraryRegistry::ActiveLibEntry_GetTexturePaths() {

		namespace fs = std::filesystem;

		std::vector<std::string> paths;

		if (!UserEntrySelected()) {
			MNEMOSY_ERROR("No active material");
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
				fs::path p = entryFolder / fs::u8path(entryName + texture_fileSuffix_albedo);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isNormalAssigned()) {
				fs::path p = entryFolder / fs::u8path(entryName + texture_fileSuffix_normal);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isRoughnessAssigned()) {
				fs::path p = entryFolder / fs::u8path(entryName + texture_fileSuffix_roughness);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isMetallicAssigned()) {
				fs::path p = entryFolder / fs::u8path(entryName + texture_fileSuffix_metallic);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isAoAssigned()) {
				fs::path p = entryFolder / fs::u8path(entryName + texture_fileSuffix_ambientOcclusion);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isEmissiveAssigned()) {
				fs::path p = entryFolder / fs::u8path(entryName + texture_fileSuffix_emissive);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isHeightAssigned()) {
				fs::path p = entryFolder / fs::u8path(entryName + texture_fileSuffix_height);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
			if (activeMat.isOpacityAssigned()) {
				fs::path p = entryFolder / fs::u8path(entryName + texture_fileSuffix_opacity);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}


			// include channel packed textures as well
			if (activeMat.HasPackedTextures) {

				if (!activeMat.PackedTexturesSuffixes.empty()) {

					for (int i = 0; i < activeMat.PackedTexturesSuffixes.size(); i++) {

						std::string filename = entryName + activeMat.PackedTexturesSuffixes[i] + texture_fileExtentionTiff;
						fs::path p = entryFolder / fs::u8path(filename);

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
				fs::path p = entryFolder / fs::u8path(filename);
				if (fs::exists(p)) {
					paths.push_back(p.generic_string());
				}
			}
		}
		// = Skybox Material
		else if (m_activeLibEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

			graphics::Skybox& skyboxMat = MnemosyEngine::GetInstance().GetScene().GetSkybox();

			if (skyboxMat.HasCubemaps()) {

				std::string filename = m_activeLibEntry->name + texture_skybox_fileSuffix_equirectangular;

				fs::path p = entryFolder / fs::u8path(filename);
				
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
		
	void MaterialLibraryRegistry::LibCollections_CreateNewEntry(const std::string& name, const std::filesystem::path& folderPath)
	{
		namespace fs = std::filesystem;
		// Make sure the folder is valid.
		
		//fs::path folderPathWide = core::StringUtils::u8path_from_path(folderPath);


		if (!fs::exists(folderPath)) {
			MNEMOSY_ERROR("Failed to create new library because path doesn't exsist, Path: {}", folderPath.generic_string());
			return;
		}


		if (!fs::is_directory(folderPath)) {
			MNEMOSY_ERROR("Failed to create new library entry because path is not a directory, Path: {}", folderPath.generic_string());
			return;
		}
			

		// folder should be empty and not contain other files.
		if (!fs::is_empty(folderPath)) {
			MNEMOSY_ERROR("Failed to create new library entry because folder contains files. Make sure to select an empty folder location. Path: {}", folderPath.generic_string());
			return;
		}


		// now we should have a valid folder loaction for the new libCollection entry.

		// First Save Current
		ActiveLibCollection_SaveToFile();

		//std::string strFolderPathWide = folderPath.generic_string();
		//std::string strFolderPathutf8 = core::StringUtils::string_fix_u8Encoding(strFolderPathWide);

		//fs::path folderPathUtf8 = fs::path(strFolderPathutf8);


		std::string uniqueCollectionName = LibCollections_MakeNameUnique(name);

		// Create lib folder inside
		fs::path libFolderPath = folderPath / fs::path("MnemosyMaterialLibrary");

		fs::create_directories(libFolderPath);

		fs::path dataFilePath = folderPath / fs::path("MnemosyMaterialLibraryData.mnsydata");


		// Construct a new empty json data file.
		LibProcedures::LibCollection_CreateNewJsonDataFile(dataFilePath);


		// add it to the list.
		LibCollection collection;
		collection.folderPath = folderPath;
		collection.name = uniqueCollectionName;

		m_libCollectionsList.push_back(collection);

		// now select the last added entry.
		LibCollections_SwitchActiveCollection(m_libCollectionsList.size() -1);

		// save lib collections
		LibCollections_SaveToFile();
	}

	void MaterialLibraryRegistry::LibCollections_CreateNewEntryFromExisting(const std::string& name,  const std::filesystem::path& dataFilePath) {

		// first see if the path is valid

		namespace fs = std::filesystem;
		// Make sure the folder is valid.
		
		// first see if the folder alongside the data file also exsists.
		fs::path libFolderPath = dataFilePath.parent_path() / fs::path("MnemosyMaterialLibrary");
		if (!fs::exists(libFolderPath)) {
			MNEMOSY_ERROR("Failed to create new library because next to the data file there exists no LibraryPathFolder called: MnemosyMaterialLibrary.");
			return;
		}

		if (!fs::exists(dataFilePath)) {
			MNEMOSY_ERROR("Failed to create new library because path doesn't exsist, Path: {}", dataFilePath.generic_string());
			return;
		}

		if (!fs::is_regular_file(dataFilePath)) {
			MNEMOSY_ERROR("Failed to create new library entry because path does not point to a mnemosy data file, Path: {}", dataFilePath.generic_string());
			return;
		}

		if (dataFilePath.extension() != ".mnsydata") {
			MNEMOSY_ERROR("Failed to create new library entry because path does not point to a mnemosy data file, Path: {}", dataFilePath.generic_string());
			return;
		}

		// lastly we can check if the data file is a LibraryData file.
		{
			bool validDataFile = false;

			nlohmann::json readFile;
		
			std::ifstream inFileStream;
			inFileStream.open(dataFilePath);
			try {
				readFile = nlohmann::json::parse(inFileStream);
			}
			catch (nlohmann::json::parse_error err) {

				MNEMOSY_ERROR("Failed to create new library entry - Error Reading Json File Contents. Message: {}", err.what());
				inFileStream.close();
				return;
			}
			inFileStream.close();
		
			if (readFile.contains(jsonLibKey_MnemosyDataFile)) {

				validDataFile = true;

				std::string dataFileIdentifier;
				try {
					dataFileIdentifier = readFile[jsonLibKey_MnemosyDataFile].get<std::string>();
				}
				catch (nlohmann::json::parse_error err) {

					validDataFile = false;
				}

				if (validDataFile)
				{
					if (dataFileIdentifier != "UserLibraryDirectoriesData") {
						validDataFile = false;
					}
				}

			}
			else {
				validDataFile = false;
			}


			if (!validDataFile) {
			
				MNEMOSY_ERROR("Failed to create new library entry because path does not point to a mnemosy data file, Path: {}", dataFilePath.generic_string());
				return;
			}
		}
		

		std::string uniqueCollectionName = LibCollections_MakeNameUnique(name);

		
		// save active collection.
		ActiveLibCollection_SaveToFile();


		LibCollection  colEntry;
		colEntry.folderPath = dataFilePath.parent_path();
		colEntry.name = uniqueCollectionName;

		m_libCollectionsList.push_back(colEntry);


		// switch to newly added collection.
		LibCollections_SwitchActiveCollection(m_libCollectionsList.size() - 1);

		LibCollections_SaveToFile();
	}

	void MaterialLibraryRegistry::LibCollections_RemoveEntryFromList(const unsigned int index)
	{

		if (!LibCollections_IsAnyActive()) {
			return;
		}

		if (index >= m_libCollectionsList.size()) {
			return;
		}

		// if user wants to delete the last entry we can just unload everyting
		if (index == 0 && m_libCollectionsList.size() == 1) {


			ActiveLibCollection_Unload();

			m_libCollection_currentSlected_id = -1;
			m_libCollectionsList.erase(m_libCollectionsList.begin() + index);
			m_libCollectionsList.clear();
			return;
		}



		int currSelected = m_libCollection_currentSlected_id;
		std::string nameOfSelected = m_libCollectionsList[m_libCollection_currentSlected_id].name;

		if (currSelected == index) {

			ActiveLibCollection_Unload();

		}

		m_libCollectionsList.erase(m_libCollectionsList.begin() + index);


		// update current selected as it may have changed when removing an entry
		if (currSelected == index) {

			// if we deleted current selection switch to selection that is now 0. it exsits because we already handled case above when deleting the last entry.
			LibCollections_SwitchActiveCollection(0);
		}
		else if (index < currSelected) {
			// update curr selected by searching for the name in the remaining list.


			for (unsigned int i = 0; i < m_libCollectionsList.size(); i++) {


				if (m_libCollectionsList[i].name == nameOfSelected) {
					m_libCollection_currentSlected_id = i;
					break;
				}
			}
		}
	}

	void MaterialLibraryRegistry::LibCollections_RenameEntry(const unsigned int index, const std::string& newName) {

		namespace fs = std::filesystem;


		if (index < 0 || index > m_libCollectionsList.size()) {
			return;
		}


		// first make sure name is unique
		std::string uniqueName = LibCollections_MakeNameUnique(newName);

		// then we have to update the internal list.
		m_libCollectionsList[index].name = uniqueName;

	}

	void MaterialLibraryRegistry::LibCollections_SwitchActiveCollection(const unsigned int index) {

		namespace fs = std::filesystem;

		if (index >= m_libCollectionsList.size() || index < 0) {
			return;
		}


		// check that the path and data file still exsist

		// folder path of the collection
		fs::path folderPath = m_libCollectionsList[index].folderPath;
		//fs::path folderPathWide = core::StringUtils::u8path_from_path(folderPath);

		if (!fs::exists(folderPath)) {

			MNEMOSY_ERROR("Faild to load library because path does not exist anymore: {}", folderPath.generic_string());
			return;
		}

		fs::path dataFilePath = folderPath / fs::path("MnemosyMaterialLibraryData.mnsydata");

		if (!fs::exists(dataFilePath)) {
			MNEMOSY_ERROR("Faild to load library because data file does not exist anymore: {}", dataFilePath.generic_string());
			return;
		}

		unsigned int current = m_libCollection_currentSlected_id;

		if (LibCollections_IsAnyActive()) {
			ActiveLibCollection_Unload();
		}


		bool success = LibCollection_LoadIntoActiveTree(folderPath);
		if (!success) {

			if (m_folderTree) {
				delete m_folderTree;
				m_folderTree = nullptr;
			}


			MNEMOSY_ERROR("Failed to switch to a library");

			// if switching failed we should switch back to the one selected before.
			if (current != -1) {
				LibCollections_SwitchActiveCollection(current);
				return;
			}

			m_libCollection_currentSlected_id = -1;
		}
		else {
			m_libCollection_currentSlected_id = index;
			OpenFolderNode(m_folderTree->GetRootPtr());
		}

	}

	std::filesystem::path MaterialLibraryRegistry::ActiveLibCollection_GetFolderPath() {

		MNEMOSY_ASSERT(LibCollections_IsAnyActive(), "Do not call this if non are active");

		MNEMOSY_ASSERT(m_libCollection_currentSlected_id < m_libCollectionsList.size(), "Selection must map to an exsiting entry!");

		return m_libCollectionsList[m_libCollection_currentSlected_id].folderPath / std::filesystem::path("MnemosyMaterialLibrary");
	}

	std::filesystem::path MaterialLibraryRegistry::ActiveLibCollection_GetDataFilePath() {

		namespace fs = std::filesystem;

		MNEMOSY_ASSERT(LibCollections_IsAnyActive(), "Do not call this if non are active");

		MNEMOSY_ASSERT(m_libCollection_currentSlected_id < m_libCollectionsList.size(), "Selection must map to an exsiting entry!");

		fs::path dataFilePath = m_libCollectionsList[m_libCollection_currentSlected_id].folderPath / std::filesystem::path("MnemosyMaterialLibraryData.mnsydata");
		

		//fs::path dataPathWide = core::StringUtils::u8path_from_path(dataFile);
		if (!fs::exists(dataFilePath)) {
			MNEMOSY_CRITICAL("Data File of a library does not exist anymore. Path: {}", dataFilePath.generic_string());
			return fs::path();
		}

		return dataFilePath;
	}

	const std::string MaterialLibraryRegistry::ActiveLibCollection_GetName()
	{
		MNEMOSY_ASSERT(m_libCollection_currentSlected_id > -1 , "Should not call this if non is selected");


		return m_libCollectionsList[m_libCollection_currentSlected_id].name;
	}

	void MaterialLibraryRegistry::ActiveLibCollection_Unload()
	{

		if (!LibCollections_IsAnyActive()) {
			return;
		}

		SetDefaultMaterial();

		ActiveLibCollection_SaveToFile();

		if (m_selectedFolderNode != nullptr) {
			MnemosyEngine::GetInstance().GetThumbnailManager().UnloadAllThumbnails();
		}

		m_selectedFolderNode = nullptr;

		if (m_folderTree) {
			m_folderTree->Shutdown();
			delete m_folderTree;
			m_folderTree = nullptr;
		}

		m_libCollection_currentSlected_id = -1;
	}

	std::string MaterialLibraryRegistry::LibCollections_MakeNameUnique(const std::string& name) {



		if (m_libCollectionsList.empty()) {
			return name;
		}

		std::string unique = name;

		// first make sure name is unique
		std::string newNameLower = core::StringUtils::ToLowerCase(name);

		
		bool nameExistsAlready = false;
		// first go through once to see if it exsists
		for (unsigned int i = 0; i < m_libCollectionsList.size(); i++) {

			std::string entryLower = core::StringUtils::ToLowerCase(m_libCollectionsList[i].name);

			if (entryLower == newNameLower) {
				nameExistsAlready = true;
				break;
			}
		}

		if (!nameExistsAlready) {
			// if it doesn't exsist we can just return it directly
			return unique;
		}
		else
		{
			// if it does exsist keep incrementing number suffix until it is not found anymore.

			std::string compare = newNameLower;
				
			unsigned int suffixCounter = 0;

			while (nameExistsAlready) {

				suffixCounter++;
					
				compare = newNameLower + "_" + std::to_string(suffixCounter);

				nameExistsAlready = false; // asume it doesn't exsist

				for (unsigned int i = 0; i < m_libCollectionsList.size(); i++) {

					std::string entryLower = core::StringUtils::ToLowerCase(m_libCollectionsList[i].name);

					if (entryLower == compare) {
						nameExistsAlready = true;
						break;
					}
				}

			}

			// use original name here for final construction to keep initial capilatizaiton.
			unique = name + "_" + std::to_string(suffixCounter);

		}
		
		return unique;
	}

	void MaterialLibraryRegistry::LibCollections_SaveToFile()
	{
		namespace fs = std::filesystem;

		
		fs::path  dataFilePath = m_fileDirectories->GetDataPath() / fs::path("LibCollections.mnsydata");

		flcrm::JsonSettings dataFile;

		bool success = false;

		dataFile.FileOpen(success, dataFilePath,jsonKey_header,"This File stores the location and names of user created libraries");

		bool anyCollectionLoaded = !m_libCollectionsList.empty();
		dataFile.WriteBool(success, "AnyLibCollectionsSaved" , anyCollectionLoaded);


		if (anyCollectionLoaded) {

			std::vector<std::string> libCollectionPaths;
			std::vector<std::string> libCollectionNames;
			
			
			for (unsigned int i = 0; i < m_libCollectionsList.size(); i++) {


				std::string path = m_libCollectionsList[i].folderPath.generic_string();

				path = core::StringUtils::string_fix_u8Encoding(path);

				libCollectionPaths.push_back(path);
				libCollectionNames.push_back(m_libCollectionsList[i].name);
			}

			dataFile.WriteVectorString(success, "LibCollectionPaths", libCollectionPaths);
			dataFile.WriteVectorString(success, "LibCollectionNames", libCollectionNames);
			dataFile.WriteInt(success, "LastLoadedCollectionId", m_libCollection_currentSlected_id);

			libCollectionPaths.clear();
			libCollectionNames.clear();
		}
		else {
			dataFile.WriteInt(success, "LastLoadedCollectionId", 0);
		}

		dataFile.FilePrettyPrintSet(true);
		dataFile.FileClose(success, dataFilePath);
	}

	void MaterialLibraryRegistry::LibCollections_LoadFromFile()
	{

		namespace fs = std::filesystem;

		if (!m_libCollectionsList.empty()) {

			m_libCollectionsList.clear();
		}


		fs::path  dataFilePath = m_fileDirectories->GetDataPath() / fs::path("LibCollections.mnsydata");

		flcrm::JsonSettings dataFile;

		bool success = false;

		dataFile.FileOpen(success, dataFilePath, jsonKey_header, "This File stores the location and names of user created libraries");


		bool anyCollectionsSaved = dataFile.ReadBool(success, "AnyLibCollectionsSaved", false, true);


		if (anyCollectionsSaved) {

			int lastLoadedId = dataFile.ReadInt(success, "LastLoadedCollectionId", -1 , true);

			std::vector<std::string> libCollectionPaths = dataFile.ReadVectorString(success, "LibCollectionPaths", std::vector<std::string>(), false);
			std::vector<std::string> libCollectionNames = dataFile.ReadVectorString(success, "LibCollectionNames", std::vector<std::string>(), false);
			
			MNEMOSY_ASSERT(libCollectionPaths.size() == libCollectionNames.size(), "These two are related an must correspond with each other");

			if (libCollectionPaths.empty()) {
				dataFile.WriteBool(success, "AnyLibCollectionsSaved", false);
				dataFile.WriteInt(success, "LastLoadedCollectionId", -1);

				m_libCollection_currentSlected_id = -1;

			}
			else {

				//m_libCollectionsList = std::vector<LibCollection>(libCollectionPaths.size());
				for (unsigned int i = 0; i < libCollectionPaths.size(); i++) {


					LibCollection l;
					l.folderPath = fs::u8path(libCollectionPaths[i]);
					l.name = libCollectionNames[i];

					bool failedToLoad = false;

					if (!fs::exists(l.folderPath)) {
						failedToLoad = true;
					}

					fs::path dataFile = l.folderPath / fs::path("MnemosyMaterialLibraryData.mnsydata");

					if (!fs::exists(dataFile)) {
						failedToLoad = true;
					}

					if (!failedToLoad) {

						// try reading the json data
						std::ifstream inputFileStream;
						inputFileStream.open(dataFile);


						try {
							nlohmann::json j = nlohmann::json::parse(inputFileStream);

						}
						catch (nlohmann::json::parse_error pErr) {

							MNEMOSY_ERROR("Failed Parse jsonFile, message: {}", pErr.what());
							failedToLoad = true;
						}

						inputFileStream.close();

					}

					if (failedToLoad) {

						MNEMOSY_ERROR("Failed to Load Material Library with Name {}: MaterialLibraryDataFile.mnsydata my be corrupted or missing \npath: {}",l.name, dataFile.generic_string());

						if (i == lastLoadedId) {
							// if the last loaded id is the one we cant load..

							if (i > 0) {
								// if we already loaded the 0 entry wihtout problems, select that one as default.
								lastLoadedId = 0;
							}
							else if (i < libCollectionPaths.size() - 1) {
								 // if we still have libraries to load set lastLoadedID to the next inline and hopefully those wont fail.
								lastLoadedId = i + 1;
							}
							else {
								// this case should only happen if we fail to load all user libraries in wich case non will be selected by default.
								lastLoadedId = -1;
							}
						}
					}
					else {
						m_libCollectionsList.push_back(l);
					}
				}

				m_libCollection_currentSlected_id = lastLoadedId;


			}


			libCollectionPaths.clear();
			libCollectionNames.clear();
		}

		dataFile.FilePrettyPrintSet(true);
		dataFile.FileClose(success, dataFilePath);

	}


} // !mnemosy::systems
