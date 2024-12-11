#include "Include/Systems/ExportManager.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#include "Include/Core/Clock.h"

#include "json.hpp"

#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/LibraryProcedures.h"

#include "Include/Systems/TextureGenerationManager.h"

#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/Skybox.h"

#include "Include/Graphics/Utils/Picture.h"

#include <glad/glad.h>

namespace mnemosy::systems
{



	void ExportManager::Init() {
		m_exportFileFormat = graphics::ImageFileFormat::MNSY_FILE_FORMAT_PNG;
		m_exportNormalFormat = graphics::MNSY_NORMAL_FORMAT_OPENGL;
		m_exportRoughnessAsSmoothness = false;

		// load user settings

		std::filesystem::path p = MnemosyEngine::GetInstance().GetFileDirectories().GetUserSettingsPath() / std::filesystem::path("exportSettings.mnsydata");

		bool success = false;

		flcrm::JsonSettings settings;
		settings.FileOpen(success, p, "Mnemosy_Settings", "Contains user settings for exporting images");


		bool exportNormalAsOpenGl = settings.ReadBool(success, "export_NormalAsOpenGl", true, true);
		bool exportRoughnessAsSmoothness = settings.ReadBool(success, "export_RoughnessAsSmoothness", false, true);
		std::string exportImageFormat = settings.ReadString(success, "export_ImageFormat", ".tif", true);

		settings.FilePrettyPrintSet(true);

		settings.FileClose(success, p);

		// apply user settings
		if (exportNormalAsOpenGl) {
			SetNormalMapExportFormat(graphics::NormalMapFormat::MNSY_NORMAL_FORMAT_OPENGL);
		}
		else {
			SetNormalMapExportFormat(graphics::NormalMapFormat::MNSY_NORMAL_FORMAT_DIRECTX);
		}

		SetExportRoughnessAsSmoothness(exportRoughnessAsSmoothness);


		graphics::ImageFileFormat imageFormat = graphics::TexUtil::get_imageFileFormat_from_fileExtentionString(exportImageFormat);
		SetExportImageFormat(imageFormat);



	}

	void ExportManager::Shutdown()
	{
		// save user settings
		std::filesystem::path p = MnemosyEngine::GetInstance().GetFileDirectories().GetUserSettingsPath() / std::filesystem::path("exportSettings.mnsydata");

		bool success = false;


		// get user settings

		bool exportNormalAsOpenGl = true;
		if (m_exportNormalFormat == graphics::NormalMapFormat::MNSY_NORMAL_FORMAT_DIRECTX) {
			exportNormalAsOpenGl = false;
		}

		bool exportRoughnessAsSmoothness = m_exportRoughnessAsSmoothness;

		std::string exportFormatExtention = graphics::TexUtil::get_string_from_imageFileFormat(m_exportFileFormat);



		flcrm::JsonSettings settings;
		settings.FileOpen(success, p, "Mnemosy_Settings", "Contains user settings for exporting images");

		settings.WriteBool(success, "export_NormalAsOpenGl", exportNormalAsOpenGl);
		settings.WriteBool(success, "export_RoughnessAsSmoothness", exportRoughnessAsSmoothness);
		settings.WriteString(success, "export_ImageFormat", exportFormatExtention);

		settings.FilePrettyPrintSet(true);
		settings.FileClose(success, p);

	}

	// Export selected textures of a material. Which textures to export should be specified in the std::vector<bool> exportTypesOrdered which need an entry for each texture type in the same order as the enum types defined in PBRTextureType in TextureDefinitions.h
	bool ExportManager::PbrMat_ExportTextures(std::filesystem::path& exportFolderPath, systems::LibEntry* libEntry, graphics::PbrMaterial& material, std::vector<bool>& exportTypesOrdered, bool exportChannelPacked) {

		namespace fs = std::filesystem;

		MNEMOSY_INFO("Exporting Material: {}, as {} using {} normal map format \nExport Path: {}", libEntry->name, graphics::TexUtil::get_string_from_imageFileFormat(m_exportFileFormat),  graphics::TexUtil::get_string_from_normalMapFormat(m_exportNormalFormat), exportFolderPath.generic_string());


		std::string fileExtention = graphics::TexUtil::get_string_from_imageFileFormat(m_exportFileFormat);

		std::string entryName = libEntry->name;
		
		// Export Albedo
		if (exportTypesOrdered[0]) {

			if (material.isAlbedoAssigned()) {

				fs::path to = exportFolderPath / fs::path(entryName + "_albedo_sRGB" + fileExtention);
				graphics::Texture& tex = material.GetAlbedoTexture();

				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(),tex.GetTextureFormat(),tex.IsHalfFloat());
				
				GLTextureExport(tex.GetID(),info);
			}
		}

		// Export Roughness
		if (exportTypesOrdered[1]) {

			if (material.isRoughnessAssigned()) {

				if (m_exportRoughnessAsSmoothness) {

					fs::path to = exportFolderPath / fs::path(entryName + "_smoothness_raw" + fileExtention);

					TextureGenerationManager& texGenerator = MnemosyEngine::GetInstance().GetTextureGenerationManager();

					texGenerator.InvertRoughness(material, to.generic_string().c_str(), true);

				}
				else {

					fs::path to = exportFolderPath / fs::path(entryName + "_roughness_raw" + fileExtention);
					graphics::Texture& tex = material.GetRoughnessTexture();

					graphics::TextureFormat f = graphics::TexUtil::get_channel_textureFormat(tex.GetTextureFormat());

					TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), f, tex.IsHalfFloat());
					
					GLTextureExport(tex.GetID(), info);
				}
			}
		}

		// Export Metallic
		if (exportTypesOrdered[2]) {

			if (material.isMetallicAssigned()) {
				fs::path to = exportFolderPath / fs::path(entryName + "_metallic_raw" + fileExtention);
				graphics::Texture& tex = material.GetMetallicTexture();

				graphics::TextureFormat f = graphics::TexUtil::get_channel_textureFormat(tex.GetTextureFormat());
				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), f, tex.IsHalfFloat());
				

				GLTextureExport(tex.GetID(), info);
			}
		}

		// Export Normal
		if (exportTypesOrdered[3]) {
			if (material.isNormalAssigned()) {

				if (m_exportNormalFormat == graphics::MNSY_NORMAL_FORMAT_OPENGL) {
					fs::path to = exportFolderPath / fs::path(entryName + "_normal_gl_raw" + fileExtention);
					graphics::Texture& tex = material.GetNormalTexture();

					// normal maps should always have RGB channels
					graphics::TextureFormat channelFormat = graphics::TexUtil::get_channel_textureFormat(tex.GetTextureFormat());
					graphics::TextureFormat format = graphics::TextureFormat::MNSY_RGB8;
					if (channelFormat == graphics::TextureFormat::MNSY_R16) {
						format = graphics::TextureFormat::MNSY_RGB16;
					}
					else if (channelFormat == graphics::TextureFormat::MNSY_R32) {
						format = graphics::TextureFormat::MNSY_RGB32;
					}


					TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), format, tex.IsHalfFloat());
					
					GLTextureExport(tex.GetID(), info);
				}
				else if (m_exportNormalFormat == graphics::MNSY_NORMAL_FORMAT_DIRECTX) {
					// Convert and then export
					TextureGenerationManager& texGenerator = MnemosyEngine::GetInstance().GetTextureGenerationManager();

					fs::path to = exportFolderPath / fs::path(entryName + "_normal_dx_raw" + fileExtention);
					texGenerator.FlipNormalMap(to.generic_string().c_str(), material, true);

				}
			}
		}

		// Export AO
		if (exportTypesOrdered[4]) {
			if (material.isAoAssigned()) {
				fs::path to = exportFolderPath / fs::path(entryName + "_ambientOcclusion_raw" + fileExtention);
				graphics::Texture& tex = material.GetAOTexture();

				graphics::TextureFormat f = graphics::TexUtil::get_channel_textureFormat(tex.GetTextureFormat());
				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), f, tex.IsHalfFloat());
				GLTextureExport(tex.GetID(), info);
			}
		}

		// Export Emissive 
		if (exportTypesOrdered[5]) {
			if (material.isEmissiveAssigned()) {

				fs::path to = exportFolderPath / fs::path(entryName + "_emissive_sRGB" + fileExtention);
				graphics::Texture& tex = material.GetEmissiveTexture();

				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), tex.GetTextureFormat(), tex.IsHalfFloat());
				
				GLTextureExport(tex.GetID(), info);
			}
		}

		// Export Height
		if (exportTypesOrdered[6]) {
			if (material.isHeightAssigned()) {
				fs::path to = exportFolderPath / fs::path(entryName + "_height_raw" + fileExtention);
				graphics::Texture& tex = material.GetHeightTexture();

				graphics::TextureFormat f = graphics::TexUtil::get_channel_textureFormat(tex.GetTextureFormat());
				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), f, tex.IsHalfFloat());
				
				GLTextureExport(tex.GetID(), info);
			}
		}

		// Export Opacity
		if (exportTypesOrdered[7]) {
			if (material.isOpacityAssigned()) {
				fs::path to = exportFolderPath / fs::path(entryName + "_opacity_raw" + fileExtention);
				graphics::Texture& tex = material.GetOpacityTexture();

				graphics::TextureFormat f = graphics::TexUtil::get_channel_textureFormat(tex.GetTextureFormat());
				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), f, tex.IsHalfFloat());
				
				GLTextureExport(tex.GetID(), info);
			}
		}
		
		// Export channel packed textures
		
		if (exportChannelPacked) {


			if (material.HasPackedTextures && !material.PackedTexturesSuffixes.empty()) {

				fs::path folderPath = systems::LibProcedures::LibEntry_GetFolderPath(libEntry);

				for (int i = 0; i < material.PackedTexturesSuffixes.size();i++) {

					std::string filnameOnDisk = entryName + material.PackedTexturesSuffixes[i] + texture_fileExtentionTiff;
					fs::path pathOnDisk = folderPath / fs::path(filnameOnDisk);


					std::string filename = entryName + material.PackedTexturesSuffixes[i] + fileExtention;
					fs::path to = exportFolderPath / fs::path(filename);

					// Since we store images as tiff files if we export as tiff we can just copy the files
					if (m_exportFileFormat == graphics::ImageFileFormat::MNSY_FILE_FORMAT_TIF) {								
						
						try {
							fs::copy_file(pathOnDisk, to);
						}
						catch (fs::filesystem_error error) {

							MNEMOSY_WARN("ExportManager::ExportMaterialTextures: System error copying files. \nError Message: {}", error.what())
						}
					}
					// otherwise we must generate a gl textuere first
					else  
					{

						graphics::PictureError err;
						graphics::PictureInfo picInfo =  graphics::Picture::ReadPicture(err,pathOnDisk.generic_string().c_str(),true,false,false);


						graphics::Texture* packedTexture = new graphics::Texture();
						packedTexture->GenerateOpenGlTexture(picInfo,false);						

						TextureExportInfo info = TextureExportInfo(to, packedTexture->GetWidth(), packedTexture->GetHeight(), packedTexture->GetTextureFormat(),packedTexture->IsHalfFloat());
						
						GLTextureExport(packedTexture->GetID(), info);						

						delete packedTexture;
						packedTexture = nullptr;
					}

				}
			}
		}

		return true;
	}

	void ExportManager::UnlitMat_ExportTextures(std::filesystem::path& exportFolderPath, systems::LibEntry* libEntry, graphics::UnlitMaterial& unlitMat) {

		namespace fs = std::filesystem;

		if (unlitMat.TextureIsAssigned()) {
			MNEMOSY_INFO("Exporting Unlit Material: {}, as {} \nExport Path: {}", libEntry->name, graphics::TexUtil::get_string_from_imageFileFormat(m_exportFileFormat), exportFolderPath.generic_string());

			std::string fileExtention = graphics::TexUtil::get_string_from_imageFileFormat(m_exportFileFormat);


			std::string filename = libEntry->name + "_tex" + fileExtention;

			fs::path finalExportPath = exportFolderPath / fs::path(filename);

			graphics::Texture& tex = unlitMat.GetTexture();

			TextureExportInfo info = TextureExportInfo(finalExportPath, tex.GetWidth(), tex.GetHeight(), tex.GetTextureFormat(), tex.IsHalfFloat());

			GLTextureExport(tex.GetID(), info);
		}
		else {

			MNEMOSY_WARN("Export Failed, no texture is assigned for Unlit Material: {}", libEntry->name);
		}
	}

	// TODO: implement
	void ExportManager::SkyboxMat_ExportTextures(std::filesystem::path& exportFolderPath, systems::LibEntry* libEntry, graphics::Skybox& skyboxMat) {
		namespace fs = std::filesystem;


		if (skyboxMat.IsColorCubeAssigned()) {


			// first load the hdr equirectangular

			fs::path equirectangularPath = systems::LibProcedures::LibEntry_GetFolderPath(libEntry) / fs::path (libEntry->name + texture_skybox_fileSuffix_equirectangular);

			if (!fs::exists(equirectangularPath)) {
				MNEMOSY_WARN("Export Failed, Equirectangular texture is missing for Skybox Material: {}, Path: {}", libEntry->name, equirectangularPath.generic_string());
				return;
			}
			

			graphics::PictureError picErr;
			graphics::PictureInfo picInfo = graphics::Picture::ReadPicture(picErr,equirectangularPath.generic_string().c_str(),true,true,true);
			if (!picErr.wasSuccessfull) {
				MNEMOSY_WARN("Export Failed, Faild to load Equirectangular texture from file for Skybox Material: {}, Path: {}", libEntry->name, equirectangularPath.generic_string());
				return;
			}

			graphics::Texture* tex = new graphics::Texture();

			tex->GenerateOpenGlTexture(picInfo,false);
			
			if (picInfo.pixels)
				free(picInfo.pixels);

			

			std::string fileExtention = graphics::TexUtil::get_string_from_imageFileFormat(m_exportFileFormat);


			std::string filename = libEntry->name + "_raw" + fileExtention;

			fs::path finalExportPath = exportFolderPath / fs::path(filename);



			TextureExportInfo info = TextureExportInfo(finalExportPath, tex->GetWidth(), tex->GetHeight(), tex->GetTextureFormat(), tex->IsHalfFloat());

			info.converExrAndHdrToLinear = true; // it should already be linear.

			GLTextureExport(tex->GetID(), info);


			delete tex;
		}
		else {

			MNEMOSY_WARN("Export Failed, no texture is assigned for Skybox Material: {}", libEntry->name);
		}



	}

	void ExportManager::GLTextureExport(const int glTextureID, TextureExportInfo& exportInfo) {

		graphics::TextureFormat format = exportInfo.textureFormat;

		graphics::ImageFileFormat fileFormat = graphics::TexUtil::get_imageFileFormat_from_fileExtentionString(exportInfo.path.extension().generic_string());

		if (fileFormat == graphics::ImageFileFormat::MNSY_FILE_FORMAT_NONE) {
			MNEMOSY_ERROR("ExportManager::GLTextureExport: file format {} is not supported",exportInfo.path.extension().generic_string());
			return;
		}


		// todo this should not be handled here but rather by the specific writer
		/*if(format == graphics::MNSY_RG8 ||format == graphics::MNSY_RG16 ||format == graphics::MNSY_RG32 ){
			MNEMOSY_ERROR("ExportManager::GLTextureExport: Export of dual channel textures is not supported");
			return;
		}*/

		uint16_t width = exportInfo.width;
		uint16_t height = exportInfo.height;



		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glTextureID);


		uint8_t numChannels, bitsPerChannel, bytesPerPixel;
		graphics::TexUtil::get_information_from_textureFormat(format, numChannels, bitsPerChannel, bytesPerPixel);
		
		
		bool isHalfFloat = false;

		// Spceial case for exporting EXR images. 8 bit data will be promoted to 16 bit and exr 16 bit is always half float data
		if (fileFormat == graphics::ImageFileFormat::MNSY_FILE_FORMAT_EXR) {

			isHalfFloat = true;
			if ((int)format <= 4) { // if its R8,RG8,RGB8 or RGBA8 promote it to 16 bit
				int form = (int)format + 4;

				format = (graphics::TextureFormat)( form);
			}
		}
		else if (fileFormat == graphics::ImageFileFormat::MNSY_FILE_FORMAT_JPG) {

			// for jpg we make sure its 8 bit
			format = (graphics::TextureFormat)numChannels;
		}
		else if (fileFormat == graphics::ImageFileFormat::MNSY_FILE_FORMAT_HDR) {
			// for HDR we make sure its 32 bit

			format = (graphics::TextureFormat)((uint8_t)(graphics::TextureFormat::MNSY_R32) + numChannels -1);
		}
		else if (fileFormat == graphics::ImageFileFormat::MNSY_FILE_FORMAT_PNG) {

			// for png we have to demote 32bit to 16 bit
			if ((uint8_t)format > 8) {
				format = (graphics::TextureFormat)((uint8_t)format-4);
			}

		}

		// we do this again here because we obviously may have changed format for some of the export cases above
		graphics::TexUtil::get_information_from_textureFormat(format, numChannels, bitsPerChannel, bytesPerPixel);
		
		// buffer for pixel data
		uint64_t bufferSize = width * height * bytesPerPixel;
		void* pixelBuffer =  malloc(bufferSize);

		uint32_t glHalfFloatType = GL_UNSIGNED_SHORT;
		if (isHalfFloat) {
			glHalfFloatType = GL_HALF_FLOAT;
		}

		//// ensure 4 byte alignment
		glPixelStorei(GL_PACK_ALIGNMENT, 4);

		switch (format) {
		// 8 bits per pixel
		case (graphics::TextureFormat::MNSY_R8):	 glGetTexImage(GL_TEXTURE_2D, 0, GL_RED , GL_UNSIGNED_BYTE, pixelBuffer); break;
		case (graphics::TextureFormat::MNSY_RG8):    glGetTexImage(GL_TEXTURE_2D, 0, GL_RG  , GL_UNSIGNED_BYTE, pixelBuffer); break;
		case (graphics::TextureFormat::MNSY_RGB8):   glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB , GL_UNSIGNED_BYTE, pixelBuffer); break;
		case (graphics::TextureFormat::MNSY_RGBA8):  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer); break;
		// 16 bits per pixel
		case (graphics::TextureFormat::MNSY_R16):    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED ,  glHalfFloatType, pixelBuffer); break;
		case (graphics::TextureFormat::MNSY_RG16):   glGetTexImage(GL_TEXTURE_2D, 0, GL_RG  ,  glHalfFloatType, pixelBuffer); break;
		case (graphics::TextureFormat::MNSY_RGB16):  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB ,  glHalfFloatType, pixelBuffer); break;
		case (graphics::TextureFormat::MNSY_RGBA16): glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA,  glHalfFloatType, pixelBuffer); break;
		// 32 bits per pixel
		case (graphics::TextureFormat::MNSY_R32):    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED , GL_FLOAT, pixelBuffer); break;
		case (graphics::TextureFormat::MNSY_RG32):	 glGetTexImage(GL_TEXTURE_2D, 0, GL_RG  , GL_FLOAT, pixelBuffer); break;
		case (graphics::TextureFormat::MNSY_RGB32):  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB , GL_FLOAT, pixelBuffer); break;
		case (graphics::TextureFormat::MNSY_RGBA32): glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixelBuffer); break;
		}

		graphics::PictureInfo info{ width,height,format,isHalfFloat,pixelBuffer };

		graphics::PictureError errorCheck;
		graphics::Picture::WritePicture(errorCheck, exportInfo.path.generic_string().c_str(), info,true,exportInfo.converExrAndHdrToLinear);

		if (!errorCheck.wasSuccessfull) {

			std::string exportFormatTxt = graphics::TexUtil::get_string_from_textureFormat(format);
			MNEMOSY_ERROR("An error occured while exporting. Format: {} {}x{}  to: {} \n Error Message: {}", exportFormatTxt, width, height, exportInfo.path.generic_string(), errorCheck.what);
		}
		
		if (pixelBuffer) {
			free(pixelBuffer);
		}
	}



} // !mnemosy::systems
