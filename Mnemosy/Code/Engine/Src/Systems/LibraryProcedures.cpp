#include "Include/Systems/LibraryProcedures.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#include "Include/Graphics/Utils/Picture.h"
#include "Include/Graphics/TextureDefinitions.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/Skybox.h"

#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/JsonKeys.h"

#include <json.hpp>
#include <fstream>
#include <thread>


namespace mnemosy::systems {


void LibProcedures::CreateDirectoryForFolderNode(systems::FolderNode* node) {
	namespace fs = std::filesystem;

	fs::path libraryDir = MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath();
	fs::path directoryPath = libraryDir / fs::path(node->pathFromRoot);

	fs::directory_entry newDir;
	try {
		newDir = fs::directory_entry(directoryPath);
	}
	catch (fs::filesystem_error error) {
		MNEMOSY_ERROR("Error creating initializing Directory {}\n Error Message: {}", directoryPath.generic_string(), error.what());
		return;
	}

	if (!newDir.exists()) {
		fs::create_directories(newDir.path());
	}
}

bool LibProcedures::CheckDataFile(const std::filesystem::path& dataFilePath)
{
	namespace fs = std::filesystem;

	fs::directory_entry dataFile = fs::directory_entry(dataFilePath);

	if (!dataFile.exists() || !dataFile.is_regular_file()) {

		MNEMOSY_WARN("Data file did Not Exist or is not a regular file: {} \nCreating new empty file at that location", dataFilePath.generic_string());
		std::ofstream file;
		file.open(dataFilePath);
		file << "";
		file.close();
		return false;
	}

	return true;
}


std::filesystem::path LibProcedures::LibEntry_GetFolderPath(systems::LibEntry* libEntry) {

	return MnemosyEngine::GetInstance().GetFileDirectories().GetLibraryDirectoryPath() / libEntry->GetPathFromRoot();
}

std::filesystem::path LibProcedures::LibEntry_GetDataFilePath(systems::LibEntry* libEntry) {
	return LibProcedures::LibEntry_GetFolderPath(libEntry) / std::filesystem::path(libEntry->name + ".mnsydata");
}

void LibProcedures::LibEntry_PbrMaterial_CreateNewDataFile( systems::LibEntry* libEntry, bool prettyPrint)
{
	std::filesystem::path materialDataFilePath = LibProcedures::LibEntry_GetDataFilePath(libEntry);

	bool success = false;

	flcrm::JsonSettings matFile;
	matFile.FileOpen(success, materialDataFilePath, jsonKey_header, jsonMatKey_description);


	matFile.WriteString(success, jsonMatKey_name, libEntry->name);
	matFile.WriteBool(success, jsonMatKey_hasChannelPacked, false);

	matFile.WriteFloat(success, jsonMatKey_albedo_r, 0.8f);
	matFile.WriteFloat(success, jsonMatKey_albedo_g, 0.8f);
	matFile.WriteFloat(success, jsonMatKey_albedo_b, 0.8f);

	matFile.WriteFloat(success, jsonMatKey_roughness, 0.1f);
	matFile.WriteFloat(success, jsonMatKey_metallic, 0.0f);

	matFile.WriteFloat(success, jsonMatKey_emission_r, 0.0f);
	matFile.WriteFloat(success, jsonMatKey_emission_g, 0.0f);
	matFile.WriteFloat(success, jsonMatKey_emission_b, 0.0f);
	matFile.WriteFloat(success, jsonMatKey_emissionStrength, 0.0f);
	matFile.WriteBool(success, jsonMatKey_useEmissiveAsMask, false);

	matFile.WriteFloat(success, jsonMatKey_normalStrength, 1.0f);
	matFile.WriteInt(success, jsonMatKey_normalMapFormat, 0);

	matFile.WriteBool(success, jsonMatKey_isSmoothness, false);
	matFile.WriteFloat(success, jsonMatKey_heightDepth, 0.0f);
	matFile.WriteFloat(success, jsonMatKey_maxHeight, 0.0f);
	matFile.WriteFloat(success, jsonMatKey_opacityThreshold, 0.5f);
	matFile.WriteBool(success, jsonMatKey_useDitheredAlpha, false);

	matFile.WriteFloat(success, jsonMatKey_uvScale_x, 1.0f);
	matFile.WriteFloat(success, jsonMatKey_uvScale_y, 1.0f);


	matFile.WriteBool(success, jsonMatKey_albedoAssigned, false);
	matFile.WriteBool(success, jsonMatKey_roughAssigned, false);
	matFile.WriteBool(success, jsonMatKey_metalAssigned, false);
	matFile.WriteBool(success, jsonMatKey_emissionAssigned, false);
	matFile.WriteBool(success, jsonMatKey_normalAssigned, false);
	matFile.WriteBool(success, jsonMatKey_aoAssigned, false);
	matFile.WriteBool(success, jsonMatKey_heightAssigned, false);
	matFile.WriteBool(success, jsonMatKey_opacityAssigned, false);

	matFile.WriteString(success, jsonMatKey_albedoPath, jsonKey_pathNotAssigned);
	matFile.WriteString(success, jsonMatKey_roughPath, jsonKey_pathNotAssigned);
	matFile.WriteString(success, jsonMatKey_metalPath, jsonKey_pathNotAssigned);
	matFile.WriteString(success, jsonMatKey_emissionPath, jsonKey_pathNotAssigned);
	matFile.WriteString(success, jsonMatKey_normalPath, jsonKey_pathNotAssigned);
	matFile.WriteString(success, jsonMatKey_aoPath, jsonKey_pathNotAssigned);
	matFile.WriteString(success, jsonMatKey_heightPath, jsonKey_pathNotAssigned);
	matFile.WriteString(success, jsonMatKey_opacityPath, jsonKey_pathNotAssigned);

	matFile.WriteString(success, jsonMatKey_thumbnailPath, std::string(libEntry->name + texture_fileSuffix_thumbnail));


	matFile.FilePrettyPrintSet(prettyPrint);

	matFile.FileClose(success, materialDataFilePath);
}


void LibProcedures::LibEntry_UnlitMaterial_CreateNewDataFile( systems::LibEntry* libEntry,bool prettyPrint) {
	MNEMOSY_ASSERT(libEntry != nullptr, "libEntry cannot be null");

	std::filesystem::path dataFilePath = LibProcedures::LibEntry_GetDataFilePath(libEntry);

	bool success = false;

	flcrm::JsonSettings file;

	file.FileOpen(success, dataFilePath, jsonKey_header, jsonKey_unlit_description);

	file.WriteString(success, jsonKey_unlit_name, libEntry->name);
	file.WriteString(success, jsonKey_unlit_texturePath, "not assigned");
	file.WriteBool(success, jsonKey_unlit_textureIsAssigned, false);
	file.WriteBool(success, jsonKey_unlit_useAlpha, false);
	file.WriteBool(success, jsonKey_unlit_useDitheredAlpha, false);
	file.WriteFloat(success, jsonKey_unlit_alphaThreshold, 0.5f);
	file.WriteFloat(success, jsonKey_unlit_uvTilingX, 1.0f);
	file.WriteFloat(success, jsonKey_unlit_uvTilingY, 1.0f);


	file.FilePrettyPrintSet(prettyPrint);
	file.FileClose(success, dataFilePath);
}

void LibProcedures::LibEntry_SkyboxMaterial_CreateNewDataFile( systems::LibEntry* libEntry, bool prettyPrint)
{
	MNEMOSY_ASSERT(libEntry != nullptr, "libEntry cannot be null");

	std::filesystem::path dataFilePath = LibProcedures::LibEntry_GetDataFilePath(libEntry);

	bool success = false;

	flcrm::JsonSettings file;

	file.FileOpen(success, dataFilePath, jsonKey_header, jsonKey_skybox_description);

	file.WriteString(success, jsonKey_skybox_name, libEntry->name);

	file.WriteFloat(success, jsonKey_skybox_exposure, 0.0f);

	file.WriteBool(success,  jsonKey_skybox_textureIsAssigned, false);
	file.WriteString(success, jsonKey_skybox_texturePath, jsonKey_pathNotAssigned);

	
	file.WriteFloat(success, jsonKey_skybox_color_r, 1.0f);
	file.WriteFloat(success, jsonKey_skybox_color_g, 1.0f);
	file.WriteFloat(success, jsonKey_skybox_color_b, 1.0f);

	file.WriteFloat(success, jsonKey_skybox_sunColor_r, 1.0f);
	file.WriteFloat(success, jsonKey_skybox_sunColor_g, 1.0f);
	file.WriteFloat(success, jsonKey_skybox_sunColor_b, 1.0f);

	file.WriteFloat(success, jsonKey_skybox_sunDir_x, 0.0f);
	file.WriteFloat(success, jsonKey_skybox_sunDir_y, 1.0f);
	file.WriteFloat(success, jsonKey_skybox_sunDir_z, 0.0f);

	file.WriteFloat(success, jsonKey_skybox_sunStrength, 1.0f);

	file.FilePrettyPrintSet(prettyPrint);
	file.FileClose(success, dataFilePath);
}

void LibProcedures::LibEntry_PbrMaterial_SaveToFile( systems::LibEntry* libEntry, graphics::PbrMaterial* pbrMat, bool prettyPrint)
{
	namespace fs = std::filesystem;

	MNEMOSY_ASSERT(libEntry && pbrMat, "They Cannot be null!");

	bool success = false;

	flcrm::JsonSettings matFile;


	fs::path dataFilePath = LibProcedures::LibEntry_GetDataFilePath(libEntry);
	matFile.FileOpen(success, dataFilePath, jsonKey_header, jsonMatKey_description);
	if (!success) {
		MNEMOSY_WARN("MaterialLibraryRegistry::SaveActiveMaterialToFile: Failed to open material data file. msg: {}", matFile.ErrorStringLastGet());
	}

	// write all material data to file.

	matFile.WriteBool(success, jsonMatKey_hasChannelPacked, pbrMat->HasPackedTextures);

	if (pbrMat->HasPackedTextures) {
		matFile.WriteVectorString(success, jsonMatKey_packedSuffixes, pbrMat->PackedTexturesSuffixes);
	}

	matFile.EntryErase(success, "1_Mnemosy_Data_File"); // keep this here for now but this key is not longer needed and depricated

	matFile.WriteString(success, jsonMatKey_name, libEntry->name);

	matFile.WriteFloat(success, jsonMatKey_albedo_r, pbrMat->Albedo.r);
	matFile.WriteFloat(success, jsonMatKey_albedo_g, pbrMat->Albedo.g);
	matFile.WriteFloat(success, jsonMatKey_albedo_b, pbrMat->Albedo.b);

	matFile.WriteFloat(success, jsonMatKey_roughness, pbrMat->Roughness);
	matFile.WriteBool(success, jsonMatKey_isSmoothness, pbrMat->IsSmoothnessTexture);

	matFile.WriteFloat(success, jsonMatKey_metallic, pbrMat->Metallic);

	matFile.WriteFloat(success, jsonMatKey_emission_r, pbrMat->Emission.r);
	matFile.WriteFloat(success, jsonMatKey_emission_g, pbrMat->Emission.g);
	matFile.WriteFloat(success, jsonMatKey_emission_b, pbrMat->Emission.b);
	matFile.WriteFloat(success, jsonMatKey_emissionStrength, pbrMat->EmissionStrength);
	matFile.WriteBool(success, jsonMatKey_useEmissiveAsMask, pbrMat->UseEmissiveAsMask);


	matFile.WriteFloat(success, jsonMatKey_normalStrength, pbrMat->NormalStrength);
	matFile.WriteInt(success, jsonMatKey_normalMapFormat,  pbrMat->GetNormalFormatAsInt()); // 0 = OpenGl, 1 = DirectX

	matFile.WriteFloat(success, jsonMatKey_heightDepth, pbrMat->HeightDepth);
	matFile.WriteFloat(success, jsonMatKey_maxHeight, pbrMat->MaxHeight);
	matFile.WriteFloat(success, jsonMatKey_opacityThreshold, pbrMat->OpacityTreshhold);
	matFile.WriteBool(success, jsonMatKey_useDitheredAlpha, pbrMat->UseDitheredAlpha);

	matFile.WriteFloat(success, jsonMatKey_uvScale_x, pbrMat->UVTiling.x);
	matFile.WriteFloat(success, jsonMatKey_uvScale_y, pbrMat->UVTiling.y);

	// loops through all possilbe texture types as defined in PBRTextureType enum
	// for each type write the texture filename and assigned bool to the file.
	for (int i = 0; i < (int)graphics::PBRTextureType::MNSY_TEXTURE_COUNT; i++) {

		graphics::PBRTextureType currentTextureType = (graphics::PBRTextureType)i;

		bool textureTypeIsAssigned = pbrMat->IsTextureTypeAssigned(currentTextureType);

		std::string jsonMatKey_path_ofTextureType = graphics::TexUtil::get_JsonMatKey_path_from_PBRTextureType(currentTextureType);
		std::string jsonMatKey_assigned_ofTextureType = graphics::TexUtil::get_JsonMatKey_assigned_from_PBRTextureType(currentTextureType);

		std::string textureFilePath = jsonKey_pathNotAssigned;

		if (textureTypeIsAssigned) {
			textureFilePath = graphics::TexUtil::get_filename_from_PBRTextureType(libEntry->name, currentTextureType);
		}

		matFile.WriteBool(success, jsonMatKey_assigned_ofTextureType, textureTypeIsAssigned);
		matFile.WriteString(success, jsonMatKey_path_ofTextureType, textureFilePath);
	}

	matFile.FilePrettyPrintSet(prettyPrint);
	
	matFile.FileClose(success, dataFilePath);
}

void LibProcedures::LibEntry_UnlitMaterial_SaveToFile(systems::LibEntry* libEntry, graphics::UnlitMaterial* unlitMat, bool prettyPrint) {
	namespace fs = std::filesystem;

	MNEMOSY_ASSERT(libEntry && unlitMat, "They cannot be null");

	bool success = false;

	flcrm::JsonSettings file;

	fs::path dataFilePath = LibProcedures::LibEntry_GetDataFilePath(libEntry);

	file.FileOpen(success, dataFilePath, jsonKey_header, jsonKey_unlit_description);
	if (!success) {
		MNEMOSY_WARN("Failed to open unlit material data file. msg: {}", file.ErrorStringLastGet());
	}

	file.WriteString(success, jsonKey_unlit_name, libEntry->name);
	file.WriteBool(success, jsonKey_unlit_useAlpha, unlitMat->useAlpha);
	file.WriteBool(success, jsonKey_unlit_useDitheredAlpha, unlitMat->useDitheredAlpha);
	file.WriteFloat(success, jsonKey_unlit_alphaThreshold, unlitMat->alphaThreshold);
	file.WriteFloat(success, jsonKey_unlit_uvTilingX, unlitMat->UVTilingX);
	file.WriteFloat(success, jsonKey_unlit_uvTilingY, unlitMat->UVTilingY);

	bool texAssigned = unlitMat->TextureIsAssigned();
	if (unlitMat->TextureIsAssigned()) {

		std::string textureFilename = libEntry->name + texture_unlit_fileSuffix;
		fs::path texPath = LibProcedures::LibEntry_GetFolderPath(libEntry) / fs::path(textureFilename);

		if (fs::exists(texPath)) {

			file.WriteBool(success, jsonKey_unlit_textureIsAssigned, true);
			file.WriteString(success, jsonKey_unlit_texturePath, textureFilename);
		}
		else {
			file.WriteBool(success, jsonKey_unlit_textureIsAssigned, false);
			file.WriteString(success, jsonKey_unlit_texturePath, jsonKey_pathNotAssigned);
		}
	}
	else {
		file.WriteBool(success, jsonKey_unlit_textureIsAssigned, false);
		file.WriteString(success, jsonKey_unlit_texturePath, jsonKey_pathNotAssigned);
	}


	file.FilePrettyPrintSet(prettyPrint);

	file.FileClose(success, dataFilePath);
}

void LibProcedures::LibEntry_SkyboxMaterial_SaveToFile( systems::LibEntry* libEntry, graphics::Skybox* skybox, bool prettyPrint)
{
	namespace fs = std::filesystem;

	MNEMOSY_ASSERT(libEntry != nullptr && skybox != nullptr, "They cannot be null");


	std::filesystem::path dataFilePath = LibProcedures::LibEntry_GetDataFilePath(libEntry);

	bool success = false;

	flcrm::JsonSettings file;

	file.FileOpen(success, dataFilePath, jsonKey_header, jsonKey_skybox_description);
	if (!success) {
		MNEMOSY_WARN("Failed to open skybox material data file. msg: {}", file.ErrorStringLastGet());
	}

	file.WriteString(success, jsonKey_skybox_name, libEntry->name);

	file.WriteFloat(success, jsonKey_skybox_exposure, skybox->exposure);

	file.WriteFloat(success, jsonKey_skybox_color_r, skybox->color.r);
	file.WriteFloat(success, jsonKey_skybox_color_g, skybox->color.g);
	file.WriteFloat(success, jsonKey_skybox_color_b, skybox->color.b);

	file.WriteFloat(success, jsonKey_skybox_sunColor_r, skybox->sunColor.r);
	file.WriteFloat(success, jsonKey_skybox_sunColor_g, skybox->sunColor.g);
	file.WriteFloat(success, jsonKey_skybox_sunColor_b, skybox->sunColor.b);

	file.WriteFloat(success, jsonKey_skybox_sunDir_x, skybox->sunDir.x);
	file.WriteFloat(success, jsonKey_skybox_sunDir_y, skybox->sunDir.y);
	file.WriteFloat(success, jsonKey_skybox_sunDir_z, skybox->sunDir.z);

	file.WriteFloat(success, jsonKey_skybox_sunStrength, skybox->sunStrength);


	if (skybox->IsTextureAssigned()) {

		// check if the img really exists

		std::string textureFilename = libEntry->name + texture_skybox_fileSuffix_equirectangular;
		fs::path texPath = LibProcedures::LibEntry_GetFolderPath(libEntry) / fs::path(textureFilename);

		if (fs::exists(texPath)) {

			file.WriteBool(success, jsonKey_skybox_textureIsAssigned, true);
			file.WriteString(success, jsonKey_skybox_texturePath, textureFilename);
		}
		else {
			file.WriteBool(success, jsonKey_skybox_textureIsAssigned, false);
			file.WriteString(success, jsonKey_skybox_texturePath, jsonKey_pathNotAssigned);
		}
	}
	else {

		file.WriteBool(success, jsonKey_skybox_textureIsAssigned, false);
		file.WriteString(success, jsonKey_skybox_texturePath, jsonKey_pathNotAssigned);
	}

	file.FilePrettyPrintSet(prettyPrint);
	file.FileClose(success, dataFilePath);
}



void LibProcedures::LibEntry_PbrMaterial_RenameFiles(LibEntry* libEntry, std::filesystem::path& entryFolderOldNamePath, std::string& oldName, bool prettyPrint) {

	// libEntry->name is already reanme to the new name but the folder has not been renamed yet

	namespace fs = std::filesystem;

	std::string finalName = libEntry->name;

	fs::path entryFolder = entryFolderOldNamePath;
	// change name of data file.
	{
		fs::path newDataFilePath = entryFolder / fs::path(finalName + ".mnsydata");
		fs::path oldDataFilePath = entryFolder / fs::path(oldName + ".mnsydata");

		bool success = false;
		flcrm::JsonSettings matFile;

		matFile.FileOpen(success, oldDataFilePath, jsonKey_header, jsonMatKey_description);
		if (!success)
		{
			MNEMOSY_ERROR("Error opening material file {} file. Message: {}", oldDataFilePath.generic_string(), matFile.ErrorStringLastGet());
			return;
		}


		matFile.WriteString(success, jsonMatKey_name, finalName);

		// Renaming all accosiated textures
		// loops through all possilbe texture types as defined in PBRTextureType enum
		for (int i = 0; i < (int)graphics::PBRTextureType::MNSY_TEXTURE_COUNT; i++) {

			std::string assignedKey = graphics::TexUtil::get_JsonMatKey_assigned_from_PBRTextureType((graphics::PBRTextureType)i);

			bool textureTypeIsAssigned = matFile.ReadBool(success, assignedKey, false, false);

			if (textureTypeIsAssigned) {

				std::string pathJsonKey = graphics::TexUtil::get_JsonMatKey_path_from_PBRTextureType((graphics::PBRTextureType)i);

				std::string oldFileName = matFile.ReadString(success, pathJsonKey, jsonKey_pathNotAssigned, false);
				fs::path oldTextureFile = entryFolder / fs::path(oldFileName);

				if (fs::exists(oldTextureFile)) { // check if the texture exists


					std::string newFileName = graphics::TexUtil::get_filename_from_PBRTextureType(finalName, (graphics::PBRTextureType)i);
					fs::path newTextureFile = entryFolder / fs::path(newFileName);

					try {
						fs::rename(oldTextureFile, newTextureFile);
					}
					catch (fs::filesystem_error e) {
						MNEMOSY_ERROR("System error renaming file. \nError message: {}", e.what());
					}



					matFile.WriteString(success, pathJsonKey, newFileName);
				}
				else {

					matFile.WriteBool(success, assignedKey, false);
					matFile.WriteString(success, pathJsonKey, jsonKey_pathNotAssigned);
				}

			}
		}

		// now we get all channelpacked textures with this material and rename them too

		bool hasPacked = matFile.ReadBool(success, jsonMatKey_hasChannelPacked, false, true);

		if (hasPacked) {

			std::vector<std::string> suffixes = matFile.ReadVectorString(success, jsonMatKey_packedSuffixes, std::vector<std::string>(), false);


			if (!suffixes.empty()) {


				// rename channel packed textures
				for (int i = 0; i < suffixes.size(); i++) {

					std::string oldFileName = oldName + suffixes[i] + texture_fileExtentionTiff;
					std::string newFileName = finalName + suffixes[i] + texture_fileExtentionTiff;

					fs::path oldFilePath = entryFolder / fs::path(oldFileName);
					fs::path newFilePath = entryFolder / fs::path(newFileName);

					try {
						fs::rename(oldFilePath, newFilePath);
					}
					catch (fs::filesystem_error e) {
						MNEMOSY_ERROR("System error renaming channelpacked texture file. \nError message: {}", e.what());
					}
				}
			}
		}

		// change name of thumbnail file
		std::string oldFileName = matFile.ReadString(success, jsonMatKey_thumbnailPath, jsonKey_pathNotAssigned, false);
		std::string newFileName = finalName + texture_fileSuffix_thumbnail;
		fs::path oldTextureFile = entryFolder / fs::path(oldFileName);
		fs::path newTextureFile = entryFolder / fs::path(newFileName);

		if (fs::exists(oldTextureFile)) {
			try { 
				fs::rename(oldTextureFile, newTextureFile);
			}
			catch (fs::filesystem_error e) { 
				MNEMOSY_ERROR("System error renaming file. \nError message: {}", e.what()); 
			}

			matFile.WriteString(success, jsonMatKey_thumbnailPath, newFileName);
		}

		// outputting updated file

		matFile.FilePrettyPrintSet(prettyPrint);
		
		matFile.FileClose(success, oldDataFilePath);

		try { fs::rename(oldDataFilePath, newDataFilePath); }
		catch (fs::filesystem_error error) { MNEMOSY_ERROR("System error renaming dataFile. \nError message: {}", error.what()); }
	}
}

void LibProcedures::LibEntry_UnlitMaterial_RenameFiles(LibEntry* libEntry, std::filesystem::path& entryFolderOldNamePath, std::string& oldName, bool prettyPrint)
{

	// libEntry->name is already renamed to the new name but the folder has not been renamed yet
	// this method renames all files stored within the folder and the data file plus values within the data file.

	namespace fs = std::filesystem;

	std::string newName = libEntry->name;

	fs::path entryFolder = entryFolderOldNamePath;


	fs::path newDataFilePath = entryFolder / fs::path(newName + ".mnsydata");
	fs::path oldDataFilePath = entryFolder / fs::path(oldName + ".mnsydata");

	bool success = false;
	flcrm::JsonSettings dataFile;

	dataFile.FileOpen(success, oldDataFilePath, jsonKey_header, jsonKey_unlit_description);
	if (!success)
	{
		MNEMOSY_ERROR("Error opening material file {} file. Message: {}", oldDataFilePath.generic_string(), dataFile.ErrorStringLastGet());
	}


	dataFile.WriteString(success, jsonKey_unlit_name, newName);

	// change name of texture if one is assigned
	bool textureAssigend = dataFile.ReadBool(success, jsonKey_unlit_textureIsAssigned,false,false);

	if (textureAssigend) {

		fs::path oldTextureFile = entryFolder / fs::path(oldName + texture_unlit_fileSuffix);

		if (fs::exists(oldTextureFile)) { // check that the file exists

			fs::path newTextureFile = entryFolder / fs::path(newName + texture_unlit_fileSuffix);

			try {
				fs::rename(oldTextureFile, newTextureFile);
			}
			catch (fs::filesystem_error e) {
				MNEMOSY_ERROR("System error renaming file. \nError message: {}", e.what());
			}


		}
		else {
			dataFile.WriteBool(success, jsonKey_unlit_textureIsAssigned,false);
			dataFile.WriteString(success, jsonKey_unlit_texturePath, jsonKey_pathNotAssigned);
		}
	}


	// change name of thumbnail file
	{
		fs::path oldTextureFile = entryFolder / fs::path(oldName + texture_fileSuffix_thumbnail);
		
		if (fs::exists(oldTextureFile)) {

			fs::path newTextureFile = entryFolder / fs::path(newName + texture_fileSuffix_thumbnail);


			try { 
				fs::rename(oldTextureFile, newTextureFile); 
			}
			catch (fs::filesystem_error e) {
				MNEMOSY_ERROR("System error renaming file. \nError message: {}", e.what()); 
			}
		}
	}

	// save data file
	dataFile.FilePrettyPrintSet(prettyPrint);

	dataFile.FileClose(success, oldDataFilePath);

	// rename data file 
	try { 
		fs::rename(oldDataFilePath, newDataFilePath); 
	}
	catch (fs::filesystem_error error) { 
		MNEMOSY_ERROR("System error renaming dataFile. \nError message: {}", error.what()); 
	}
}


void LibProcedures::LibEntry_SkyboxMaterial_RenameFiles(LibEntry* libEntry, std::filesystem::path& entryFolderOldNamePath, std::string& oldName, bool prettyPrint) {
	// libEntry->name is already renamed to the new name but the folder has not been renamed yet
	// this method renames all files stored within the folder and the data file plus values within the data file.

	namespace fs = std::filesystem;

	std::string newName = libEntry->name;

	fs::path entryFolder = entryFolderOldNamePath;


	fs::path newDataFilePath = entryFolder / fs::path(newName + ".mnsydata");
	fs::path oldDataFilePath = entryFolder / fs::path(oldName + ".mnsydata");

	bool success = false;
	flcrm::JsonSettings dataFile;

	dataFile.FileOpen(success, oldDataFilePath, jsonKey_header, jsonKey_skybox_description);
	if (!success) {
		MNEMOSY_ERROR("Error opening material file {} file. Message: {}", oldDataFilePath.generic_string(), dataFile.ErrorStringLastGet());
	}

	dataFile.WriteString(success, jsonKey_skybox_name, newName);

	
	// change name of textures if they exist
	bool textureAssigend = dataFile.ReadBool(success, jsonKey_skybox_textureIsAssigned, false, true);

	if (textureAssigend) {

		fs::path oldTextureFile = entryFolder / fs::path(oldName + texture_skybox_fileSuffix_equirectangular);

		// rename hdr equirectangular img
		if (fs::exists(oldTextureFile)) { // check that the file exists

			fs::path newTextureFile = entryFolder / fs::path(newName + texture_skybox_fileSuffix_equirectangular);

			try {
				fs::rename(oldTextureFile, newTextureFile);
			}
			catch (fs::filesystem_error e) {
				MNEMOSY_ERROR("System error renaming file. \nError message: {}", e.what());
			}
		}
		else {
			dataFile.WriteBool(success, jsonKey_skybox_textureIsAssigned, false);
			dataFile.WriteString(success, jsonKey_skybox_texturePath, jsonKey_pathNotAssigned);
		}


			// rename cubeColor file
		{
			fs::path cubeColorFile_old = entryFolder / fs::path(oldName + texture_skybox_fileSuffix_cubeColor);

			if (fs::exists(cubeColorFile_old)) {

				fs::path cubeColorFile_new = entryFolder / fs::path(newName + texture_skybox_fileSuffix_cubeColor);
				try {
					fs::rename(cubeColorFile_old, cubeColorFile_new);
				}
				catch (fs::filesystem_error e) {
					MNEMOSY_ERROR("System error renaming file. \nError message: {}", e.what());
				}
			}
		}

		// rename cubePrefilter file
		{
			fs::path cubePrefilterFile_old = entryFolder / fs::path(oldName + texture_skybox_fileSuffix_cubePrefilter);

			if (fs::exists(cubePrefilterFile_old)) {

				fs::path cubePrefilterFile_new = entryFolder / fs::path(newName + texture_skybox_fileSuffix_cubePrefilter);
				try {
					fs::rename(cubePrefilterFile_old, cubePrefilterFile_new);
				}
				catch (fs::filesystem_error e) {
					MNEMOSY_ERROR("System error renaming file. \nError message: {}", e.what());
				}
			}
		}

		// rename cubeIrradiance file
		{
			fs::path cubeIrradianceFile_old = entryFolder / fs::path(oldName + texture_skybox_fileSuffix_cubeIrradiance);

			if (fs::exists(cubeIrradianceFile_old)) {

				fs::path cubeIrradianceFile_new = entryFolder / fs::path(newName + texture_skybox_fileSuffix_cubeIrradiance);
				try {
					fs::rename(cubeIrradianceFile_old, cubeIrradianceFile_new);
				}
				catch (fs::filesystem_error e) {
					MNEMOSY_ERROR("System error renaming file. \nError message: {}", e.what());
				}
			}
		}


	}


	// todo thumbnail rename

	// change name of thumbnail file
	{
		fs::path oldThumbFile = entryFolder / fs::path(oldName + texture_fileSuffix_thumbnail);

		if (fs::exists(oldThumbFile)) {

			fs::path newThumbFile = entryFolder / fs::path(newName + texture_fileSuffix_thumbnail);


			try {
				fs::rename(oldThumbFile, newThumbFile);
			}
			catch (fs::filesystem_error e) {
				MNEMOSY_ERROR("System error renaming file. \nError message: {}", e.what());
			}
		}
	}

	// save data file
	dataFile.FilePrettyPrintSet(prettyPrint);

	dataFile.FileClose(success, oldDataFilePath);

	// rename data file 
	try {
		fs::rename(oldDataFilePath, newDataFilePath);
	}
	catch (fs::filesystem_error error) {
		MNEMOSY_ERROR("System error renaming dataFile. \nError message: {}", error.what());
	}

}


graphics::PbrMaterial* LibProcedures::LibEntry_PbrMaterial_LoadFromFile_Multithreaded(systems::LibEntry* libEntry, bool prettyPrint)
{
	namespace fs = std::filesystem;


	// load json file
	fs::path materialDir = LibProcedures::LibEntry_GetFolderPath(libEntry);
	fs::path dataFile = LibProcedures::LibEntry_GetDataFilePath(libEntry);


	bool success = false;

	flcrm::JsonSettings matFile;
	matFile.FileOpen(success, dataFile, jsonKey_header, jsonMatKey_description);
	if (!success) {
		MNEMOSY_WARN("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded: Error opening data file: {} \nMessage: {}", dataFile.generic_string(), matFile.ErrorStringLastGet());
		//return nullptr;
	}


	// Check if all channelpacked textures still exist at their location. if not update the material file
	{
		bool hasChannelPacked = matFile.ReadBool(success, jsonMatKey_hasChannelPacked, false, false);
		if (hasChannelPacked) {
			std::vector<std::string> packSuffixes = matFile.ReadVectorString(success, jsonMatKey_packedSuffixes, std::vector<std::string>(), false);
			if (!packSuffixes.empty()) {

				std::vector<std::string> packSuffixesCopy;
				for (int i = 0; i < packSuffixes.size(); i++) {

					fs::path packedTexPath = materialDir / fs::path(libEntry->name + packSuffixes[i] + ".tif");

					if (fs::exists(packedTexPath)) {

						packSuffixesCopy.push_back(packSuffixes[i]);
					}
				}

				if (!packSuffixesCopy.empty()) {
					matFile.WriteVectorString(success, jsonMatKey_packedSuffixes, packSuffixesCopy);

					packSuffixesCopy.clear();
				}
				else {
					matFile.WriteBool(success, jsonMatKey_hasChannelPacked, false);
					matFile.EntryErase(success, jsonMatKey_packedSuffixes);
				}

			}
			else {
				matFile.WriteBool(success, jsonMatKey_hasChannelPacked, false);
			}

			packSuffixes.clear();
		}
	}

	// First kick off a thread for each assigned texture to load

	// Start a thread for each assigned texture to load it into memory of PictureInfo struct.
	// Then join threads upload to openGl on the main thread.
	// openGl calls must all be from the main thread so we have to do it like this
	// also check if the textture files actually exist and update acordingly.

	// Albedo Map
	bool albedoAssigned = matFile.ReadBool(success, jsonMatKey_albedoAssigned, false, false);

	std::thread thread_load_albedo;
	graphics::PictureError albedo_picErr;
	graphics::PictureInfo albedo_picInfo;
	if (albedoAssigned) {

		std::string path = materialDir.generic_string() + "/" + matFile.ReadString(success, jsonMatKey_albedoPath, jsonKey_pathNotAssigned, false);

		if (!fs::exists(path)) {
			matFile.WriteString(success, jsonMatKey_albedoPath, jsonKey_pathNotAssigned);
			matFile.WriteBool(success, jsonMatKey_albedoAssigned, false);
			albedoAssigned = false;
		}
		else {
			thread_load_albedo = std::thread(&graphics::Picture::ReadPicture_PbrThreaded, std::ref(albedo_picErr), std::ref(albedo_picInfo), path, true, graphics::PBRTextureType::MNSY_TEXTURE_ALBEDO);
		}
	}


	// Roughness Map
	bool roughnessAssigned = matFile.ReadBool(success, jsonMatKey_roughAssigned, false, false);
	graphics::PictureError roughness_picErr;
	graphics::PictureInfo roughness_picInfo;
	std::thread thread_load_roughness;

	if (roughnessAssigned) {

		std::string path = materialDir.generic_string() + "/" + matFile.ReadString(success, jsonMatKey_roughPath, jsonKey_pathNotAssigned, false);

		if (!fs::exists(path)) {
			matFile.WriteString(success, jsonMatKey_roughPath, jsonKey_pathNotAssigned);
			matFile.WriteBool(success, jsonMatKey_roughAssigned, false);
			roughnessAssigned = false;
		}
		else {
			thread_load_roughness = std::thread(&graphics::Picture::ReadPicture_PbrThreaded, std::ref(roughness_picErr), std::ref(roughness_picInfo), path, true, graphics::PBRTextureType::MNSY_TEXTURE_ROUGHNESS);
		}
	}

	// Metallic Map
	bool metallicAssigned = matFile.ReadBool(success, jsonMatKey_metalAssigned, false, false);
	graphics::PictureError metallic_picErr;
	graphics::PictureInfo  metallic_picInfo;
	std::thread thread_load_metallic;
	if (metallicAssigned) {
		std::string path = materialDir.generic_string() + "/" + matFile.ReadString(success, jsonMatKey_metalPath, jsonKey_pathNotAssigned, false);

		if (!fs::exists(path)) {
			matFile.WriteString(success, jsonMatKey_metalPath, jsonKey_pathNotAssigned);
			matFile.WriteBool(success, jsonMatKey_metalAssigned, false);
			metallicAssigned = false;
		}
		else {
			thread_load_metallic = std::thread(&graphics::Picture::ReadPicture_PbrThreaded, std::ref(metallic_picErr), std::ref(metallic_picInfo), path, true, graphics::PBRTextureType::MNSY_TEXTURE_METALLIC);
		}
	}

	// Emissive Map
	bool emissiveAssigned = matFile.ReadBool(success, jsonMatKey_emissionAssigned, false, false);
	graphics::PictureError emissive_picErr;
	graphics::PictureInfo  emissive_picInfo;
	std::thread thread_load_emissive;
	if (emissiveAssigned) {

		std::string path = materialDir.generic_string() + "/" + matFile.ReadString(success, jsonMatKey_emissionPath, jsonKey_pathNotAssigned, false);

		if (!fs::exists(path)) {
			matFile.WriteString(success, jsonMatKey_emissionPath, jsonKey_pathNotAssigned);
			matFile.WriteBool(success, jsonMatKey_emissionAssigned, false);
			emissiveAssigned = false;
		}
		else {
			thread_load_emissive = std::thread(&graphics::Picture::ReadPicture_PbrThreaded, std::ref(emissive_picErr), std::ref(emissive_picInfo), path, true, graphics::PBRTextureType::MNSY_TEXTURE_EMISSION);
		}

	}

	// Normal Map
	bool normalAssigned = matFile.ReadBool(success, jsonMatKey_normalAssigned, false, false);
	graphics::PictureError normal_picErr;
	graphics::PictureInfo  normal_picInfo;

	std::thread thread_load_normal;
	if (normalAssigned) {
		std::string path = materialDir.generic_string() + "/" + matFile.ReadString(success, jsonMatKey_normalPath, jsonKey_pathNotAssigned, false);

		if (!fs::exists(path)) {
			matFile.WriteString(success, jsonMatKey_normalPath, jsonKey_pathNotAssigned);
			matFile.WriteBool(success, jsonMatKey_normalAssigned, false);
			normalAssigned = false;
		}
		else {
			thread_load_normal = std::thread(&graphics::Picture::ReadPicture_PbrThreaded, std::ref(normal_picErr), std::ref(normal_picInfo), path, true, graphics::PBRTextureType::MNSY_TEXTURE_NORMAL);
		}

	}

	// AO map
	bool aoAssigned = matFile.ReadBool(success, jsonMatKey_aoAssigned, false, false);
	graphics::PictureError ao_picErr;
	graphics::PictureInfo  ao_picInfo;
	std::thread thread_load_ao;
	if (aoAssigned) {

		std::string path = materialDir.generic_string() + "/" + matFile.ReadString(success, jsonMatKey_aoPath, jsonKey_pathNotAssigned, false);

		if (!fs::exists(path)) {
			matFile.WriteString(success, jsonMatKey_aoPath, jsonKey_pathNotAssigned);
			matFile.WriteBool(success, jsonMatKey_aoAssigned, false);
			aoAssigned = false;
		}
		else {
			thread_load_ao = std::thread(&graphics::Picture::ReadPicture_PbrThreaded, std::ref(ao_picErr), std::ref(ao_picInfo), path, true, graphics::PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION);
		}
	}

	// Height map
	bool heightAssigned = matFile.ReadBool(success, jsonMatKey_heightAssigned, false, false);
	graphics::PictureError height_picErr;
	graphics::PictureInfo  height_picInfo;
	std::thread thread_load_height;
	if (heightAssigned) {
		std::string path = materialDir.generic_string() + "/" + matFile.ReadString(success, jsonMatKey_heightPath, jsonKey_pathNotAssigned, false);

		if (!fs::exists(path)) {
			matFile.WriteString(success, jsonMatKey_heightPath, jsonKey_pathNotAssigned);
			matFile.WriteBool(success, jsonMatKey_heightAssigned, false);
			heightAssigned = false;
		}
		else {
			thread_load_height = std::thread(&graphics::Picture::ReadPicture_PbrThreaded, std::ref(height_picErr), std::ref(height_picInfo), path, true, graphics::PBRTextureType::MNSY_TEXTURE_HEIGHT);
		}
	}

	// Opacity Map
	bool opacityAssigned = matFile.ReadBool(success, jsonMatKey_opacityAssigned, false, false);
	graphics::PictureError opacity_picErr;
	graphics::PictureInfo  opacity_picInfo;
	std::thread thread_load_opacity;
	if (opacityAssigned) {
		std::string path = materialDir.generic_string() + "/" + matFile.ReadString(success, jsonMatKey_opacityPath, jsonKey_pathNotAssigned, false);

		if (!fs::exists(path)) {
			matFile.WriteString(success, jsonMatKey_opacityPath, jsonKey_pathNotAssigned);
			matFile.WriteBool(success, jsonMatKey_opacityAssigned, false);
			opacityAssigned = false;
		}
		else {
			thread_load_opacity = std::thread(&graphics::Picture::ReadPicture_PbrThreaded, std::ref(opacity_picErr), std::ref(opacity_picInfo), path, true, graphics::PBRTextureType::MNSY_TEXTURE_OPACITY);
		}
	}

	graphics::PbrMaterial* mat = new graphics::PbrMaterial();
	// Once all threads are going we let the main thread do the rest of the work
	{
		//mat->Name = matFile.ReadString(success, jsonMatKey_name, libEntry->name, true);

		mat->HasPackedTextures = matFile.ReadBool(success, jsonMatKey_hasChannelPacked, false, true);

		if (mat->HasPackedTextures) {

			mat->PackedTexturesSuffixes = matFile.ReadVectorString(success, jsonMatKey_packedSuffixes, std::vector<std::string>(), false);
		}

		mat->Albedo.r = matFile.ReadFloat(success, jsonMatKey_albedo_r, 0.8f, true);
		mat->Albedo.g = matFile.ReadFloat(success, jsonMatKey_albedo_g, 0.8f, true);
		mat->Albedo.b = matFile.ReadFloat(success, jsonMatKey_albedo_b, 0.8f, true);
		mat->Roughness = matFile.ReadFloat(success, jsonMatKey_roughness, 0.1f, true);
		mat->IsSmoothnessTexture = matFile.ReadBool(success, jsonMatKey_isSmoothness, false, true);

		mat->Metallic = matFile.ReadFloat(success, jsonMatKey_metallic, 0.0f, true);

		mat->Emission.r = matFile.ReadFloat(success, jsonMatKey_emission_r, 0.0f, true);
		mat->Emission.g = matFile.ReadFloat(success, jsonMatKey_emission_g, 0.0f, true);
		mat->Emission.b = matFile.ReadFloat(success, jsonMatKey_emission_b, 0.0f, true);
		mat->EmissionStrength = matFile.ReadFloat(success, jsonMatKey_emissionStrength, 0.0f, true);
		mat->UseEmissiveAsMask = matFile.ReadBool(success, jsonMatKey_useEmissiveAsMask, false, true);

		mat->NormalStrength = matFile.ReadFloat(success, jsonMatKey_normalStrength, 1.0f, true);

		int normalFormat = matFile.ReadInt(success, jsonMatKey_normalMapFormat, 0, true);
		if (normalFormat == 0) {
			mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGL);
		}
		else if (normalFormat == 1) {
			mat->SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_DIRECTX);
		}

		mat->HeightDepth = matFile.ReadFloat(success, jsonMatKey_heightDepth, 0.0f, true);
		mat->MaxHeight = matFile.ReadFloat(success, jsonMatKey_maxHeight, 0.0f, true);
		mat->OpacityTreshhold = matFile.ReadFloat(success, jsonMatKey_opacityThreshold, 0.5f, true);
		mat->UseDitheredAlpha = matFile.ReadBool(success, jsonMatKey_useDitheredAlpha, false, true);

		mat->UVTiling.x = matFile.ReadFloat(success, jsonMatKey_uvScale_x, 1.0f, true);
		mat->UVTiling.y = matFile.ReadFloat(success, jsonMatKey_uvScale_y, 1.0f, true);


		matFile.FilePrettyPrintSet(prettyPrint);
		matFile.FileClose(success, dataFile);
	}

	// == join threads
	if (albedoAssigned) {
		thread_load_albedo.join();

		if (albedo_picErr.wasSuccessfull) {

			graphics::Texture* albedoTex = new graphics::Texture();
			albedoTex->GenerateOpenGlTexture(albedo_picInfo, true);
			mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_ALBEDO, albedoTex);
			free(albedo_picInfo.pixels);
		}
		else {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading albedo Texture \nMessage: {}", albedo_picErr.what);
		}
	}

	if (roughnessAssigned) {
		thread_load_roughness.join();
		if (roughness_picErr.wasSuccessfull) {
			graphics::Texture* roughnessTex = new graphics::Texture();
			roughnessTex->GenerateOpenGlTexture(roughness_picInfo, true);
			mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_ROUGHNESS, roughnessTex);
			free(roughness_picInfo.pixels);
		}
		else {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading roughness Texture \nMessage: {}", roughness_picErr.what);
		}
	}


	if (metallicAssigned) {
		thread_load_metallic.join();
		if (metallic_picErr.wasSuccessfull) {
			graphics::Texture* metallicTex = new graphics::Texture();
			metallicTex->GenerateOpenGlTexture(metallic_picInfo, true);
			mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_METALLIC, metallicTex);
			free(metallic_picInfo.pixels);
		}
		else {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading Metallic Texture \nMessage: {}", metallic_picErr.what);
		}
	}

	if (emissiveAssigned) {
		thread_load_emissive.join();
		if (emissive_picErr.wasSuccessfull) {
			graphics::Texture* emissionTex = new graphics::Texture();
			emissionTex->GenerateOpenGlTexture(emissive_picInfo, true);
			mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_EMISSION, emissionTex);
			free(emissive_picInfo.pixels);
		}
		else {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading Emission Texture \nMessage: {}", emissive_picErr.what);
		}
	}

	if (normalAssigned) {
		thread_load_normal.join();
		if (emissive_picErr.wasSuccessfull) {
			graphics::Texture* normalTex = new graphics::Texture();
			normalTex->GenerateOpenGlTexture(normal_picInfo, true);
			mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_NORMAL, normalTex);
			free(normal_picInfo.pixels);
		}
		else {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading Normal Texture \nMessage: {}", normal_picErr.what);
		}
	}

	if (aoAssigned) {
		thread_load_ao.join();
		if (ao_picErr.wasSuccessfull) {
			graphics::Texture* aoTex = new graphics::Texture();
			aoTex->GenerateOpenGlTexture(ao_picInfo, true);
			mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION, aoTex);
			free(ao_picInfo.pixels);
		}
		else {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading AmbientOcclusion Texture \nMessage: {}", ao_picErr.what);
		}
	}

	if (heightAssigned) {
		thread_load_height.join();
		if (height_picErr.wasSuccessfull) {
			graphics::Texture* heightTex = new graphics::Texture();
			heightTex->GenerateOpenGlTexture(height_picInfo, true);
			mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_HEIGHT, heightTex);
			free(height_picInfo.pixels);
		}
		else {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading Height Texture \nMessage: {}", height_picErr.what);
		}
	}

	if (opacityAssigned) {
		thread_load_opacity.join();

		if (opacity_picErr.wasSuccessfull) {
			graphics::Texture* opacityTex = new graphics::Texture();
			opacityTex->GenerateOpenGlTexture(opacity_picInfo, true);
			mat->assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_OPACITY, opacityTex);
			free(opacity_picInfo.pixels);
		}
		else {
			MNEMOSY_ERROR("MaterialLibraryRegistry::LoadActiveMaterialFromFile_Multithreaded:: Error loading Opacity Texture \nMessage: {}", opacity_picErr.what);
		}
	}

	return mat;
}


graphics::UnlitMaterial* LibProcedures::LibEntry_UnlitMaterial_LoadFromFile(systems::LibEntry* libEntry, bool prettyPrint) {
	namespace fs = std::filesystem;


	fs::path dataFilePath = LibProcedures::LibEntry_GetDataFilePath(libEntry);


	bool success = false;

	flcrm::JsonSettings file;

	file.FileOpen(success, dataFilePath, jsonKey_header, jsonKey_unlit_description);
	if (!success) {
		MNEMOSY_WARN("MaterialLibraryRegistry::LoadUnlitMaterialFromFile: Error opening data file: {} \nMessage: {}", dataFilePath.generic_string(), file.ErrorStringLastGet());
	}

	//std::string name = file.ReadString(success, jsonKey_unlit_name, libEntry->name, true);

	bool useAlpha = file.ReadBool(success, jsonKey_unlit_useAlpha, false, true);
	bool useDitheredAlpha = file.ReadBool(success, jsonKey_unlit_useDitheredAlpha, false, true);
	float alphaThreshold = file.ReadFloat(success, jsonKey_unlit_alphaThreshold, 0.5f, true);
	bool textureIsAssiged = file.ReadBool(success, jsonKey_unlit_textureIsAssigned, false, true);
	std::string textureFilename = file.ReadString(success, jsonKey_unlit_texturePath, "not assigned", false);

	graphics::UnlitMaterial* unlitMat = new graphics::UnlitMaterial();

	unlitMat->useAlpha = useAlpha;
	unlitMat->useDitheredAlpha = useDitheredAlpha;
	unlitMat->alphaThreshold = alphaThreshold;
	unlitMat->UVTilingX = file.ReadFloat(success, jsonKey_unlit_uvTilingX, 1.0f, true);
	unlitMat->UVTilingY = file.ReadFloat(success, jsonKey_unlit_uvTilingY, 1.0f, true);

	// load texture if it exists if not update the data file directly

	if (textureIsAssiged) {

		bool textureReadSuccess = false;

		// load texture from file.
		graphics::PictureInfo picInfo;
		graphics::PictureError picError;

		fs::path texturePath = LibEntry_GetFolderPath(libEntry) / fs::path(textureFilename);


		if (fs::exists(texturePath)) {

			picInfo = graphics::Picture::ReadPicture(picError, texturePath.generic_string().c_str(), true,true,true);
			if (!picError.wasSuccessfull) {

				MNEMOSY_WARN("Error reading texture file of unlit material {} \nMessage {}", libEntry->name, picError.what);

			}
			else {

				graphics::Texture* tex = new graphics::Texture();
				tex->GenerateOpenGlTexture(picInfo, true);
				unlitMat->AssignTexture(tex);
				textureReadSuccess = true;

				if (picInfo.pixels)
					free(picInfo.pixels);
			}
		}


		if (!textureReadSuccess) {

			file.WriteBool(success, jsonKey_unlit_textureIsAssigned, false);
			file.WriteString(success, jsonKey_unlit_texturePath, "not Assigned");
		}

	}

	file.FilePrettyPrintSet(prettyPrint);
	file.FileClose(success, dataFilePath);

	return unlitMat;
}


// TODO: implement
graphics::Skybox* LibProcedures::LibEntry_SkyboxMaterial_LoadFromFile(systems::LibEntry* libEntry, bool prettyPrint)
{
	return nullptr;
}




} // !namespace mnemosy::systems