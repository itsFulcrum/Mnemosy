#include "Include/Systems/ExportManager.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"
#include "json.hpp"

#include "Include/Systems/TextureGenerationManager.h"

#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Material.h"

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
	bool ExportManager::ExportMaterialTextures(std::filesystem::path& exportPath, std::filesystem::path& materialFolderPath, graphics::Material& material, std::vector<bool>& exportTypesOrdered, bool exportChannelPacked) {

		namespace fs = std::filesystem;

		MNEMOSY_INFO("Exporting Material: {}, as {} using {} normal map format \nExport Path: {}", material.Name, graphics::TexUtil::get_string_from_imageFileFormat(m_exportFileFormat),  graphics::TexUtil::get_string_from_normalMapFormat(m_exportNormalFormat), exportPath.generic_string());


		std::string fileExtention = graphics::TexUtil::get_string_from_imageFileFormat(m_exportFileFormat);

		// TODO: store in what format a texture was saved and use that to determine how to export it. at the moment all albedo are just always exported as Rgba8
		// Export Albedo
		if (exportTypesOrdered[0]) {

			if (material.isAlbedoAssigned()) {

				fs::path to = exportPath / fs::path(material.Name + "_albedo_sRGB" + fileExtention);
				graphics::Texture& tex = material.GetAlbedoTexture();

				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(),tex.GetTextureFormat(),tex.IsHalfFloat());
				
				GLTextureExport(tex.GetID(),info,graphics::PBRTextureType::MNSY_TEXTURE_ALBEDO);
			}
		}

		// Export Roughness
		if (exportTypesOrdered[1]) {

			if (material.isRoughnessAssigned()) {

				if (m_exportRoughnessAsSmoothness) {

					fs::path to = exportPath / fs::path(material.Name + "_smoothness_raw" + fileExtention);

					TextureGenerationManager& texGenerator = MnemosyEngine::GetInstance().GetTextureGenerationManager();

					texGenerator.InvertRoughness(material, to.generic_string().c_str(), true);

				}
				else {

					fs::path to = exportPath / fs::path(material.Name + "_roughness_raw" + fileExtention);
					graphics::Texture& tex = material.GetRoughnessTexture();

					graphics::TextureFormat f = graphics::TexUtil::get_channel_textureFormat(tex.GetTextureFormat());

					TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), f, tex.IsHalfFloat());
					
					GLTextureExport(tex.GetID(), info, graphics::PBRTextureType::MNSY_TEXTURE_ROUGHNESS);
				}
			}
		}

		// Export Metallic
		if (exportTypesOrdered[2]) {

			if (material.isMetallicAssigned()) {
				fs::path to = exportPath / fs::path(material.Name + "_metallic_raw" + fileExtention);
				graphics::Texture& tex = material.GetMetallicTexture();

				graphics::TextureFormat f = graphics::TexUtil::get_channel_textureFormat(tex.GetTextureFormat());
				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), f, tex.IsHalfFloat());
				

				GLTextureExport(tex.GetID(), info, graphics::PBRTextureType::MNSY_TEXTURE_METALLIC);
			}
		}

		// Export Normal
		if (exportTypesOrdered[3]) {
			if (material.isNormalAssigned()) {

				if (m_exportNormalFormat == graphics::MNSY_NORMAL_FORMAT_OPENGL) {
					fs::path to = exportPath / fs::path(material.Name + "_normal_gl_raw" + fileExtention);
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
					
					GLTextureExport(tex.GetID(), info, graphics::PBRTextureType::MNSY_TEXTURE_NORMAL);
				}
				else if (m_exportNormalFormat == graphics::MNSY_NORMAL_FORMAT_DIRECTX) {
					// Convert and then export
					TextureGenerationManager& texGenerator = MnemosyEngine::GetInstance().GetTextureGenerationManager();

					fs::path to = exportPath / fs::path(material.Name + "_normal_dx_raw" + fileExtention);
					texGenerator.FlipNormalMap(to.generic_string().c_str(), material, true);

				}
			}
		}

		// Export AO
		if (exportTypesOrdered[4]) {
			if (material.isAoAssigned()) {
				fs::path to = exportPath / fs::path(material.Name + "_ambientOcclusion_raw" + fileExtention);
				graphics::Texture& tex = material.GetAOTexture();

				graphics::TextureFormat f = graphics::TexUtil::get_channel_textureFormat(tex.GetTextureFormat());
				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), f, tex.IsHalfFloat());
				GLTextureExport(tex.GetID(), info, graphics::PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION);
			}
		}

		// Export Emissive 
		if (exportTypesOrdered[5]) {
			if (material.isEmissiveAssigned()) {

				fs::path to = exportPath / fs::path(material.Name + "_emissive_sRGB" + fileExtention);
				graphics::Texture& tex = material.GetEmissiveTexture();

				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), tex.GetTextureFormat(), tex.IsHalfFloat());
				
				GLTextureExport(tex.GetID(), info, graphics::PBRTextureType::MNSY_TEXTURE_EMISSION);
			}
		}

		// Export Height
		if (exportTypesOrdered[6]) {
			if (material.isHeightAssigned()) {
				fs::path to = exportPath / fs::path(material.Name + "_height_raw" + fileExtention);
				graphics::Texture& tex = material.GetHeightTexture();

				graphics::TextureFormat f = graphics::TexUtil::get_channel_textureFormat(tex.GetTextureFormat());
				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), f, tex.IsHalfFloat());
				
				GLTextureExport(tex.GetID(), info, graphics::PBRTextureType::MNSY_TEXTURE_HEIGHT);
			}
		}

		// Export Opacity
		if (exportTypesOrdered[7]) {
			if (material.isOpacityAssigned()) {
				fs::path to = exportPath / fs::path(material.Name + "_opacity_raw" + fileExtention);
				graphics::Texture& tex = material.GetOpacityTexture();

				graphics::TextureFormat f = graphics::TexUtil::get_channel_textureFormat(tex.GetTextureFormat());
				TextureExportInfo info = TextureExportInfo(to, tex.GetWidth(), tex.GetHeight(), f, tex.IsHalfFloat());
				
				GLTextureExport(tex.GetID(), info, graphics::PBRTextureType::MNSY_TEXTURE_OPACITY);
			}
		}
		
		// Export channel packed textures
		
		if (exportChannelPacked) {


			if (material.HasPackedTextures && !material.PackedTexturesSuffixes.empty()) {



				for (int i = 0; i < material.PackedTexturesSuffixes.size();i++) {

					std::string filnameOnDisk = material.Name + material.PackedTexturesSuffixes[i] + texture_textureFileType;
					fs::path pathOnDisk = materialFolderPath / fs::path(filnameOnDisk);


					std::string filename = material.Name + material.PackedTexturesSuffixes[i] + fileExtention;
					fs::path to = exportPath / fs::path(filename);

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
						graphics::PictureInfo picInfo =  graphics::Picture::ReadPicture(err,pathOnDisk.generic_string().c_str(),true,graphics::PBRTextureType::MNSY_TEXTURE_NONE);


						graphics::Texture* packedTexture = new graphics::Texture();
						packedTexture->GenerateOpenGlTexture(picInfo,false);						

						TextureExportInfo info = TextureExportInfo(to.generic_string(), packedTexture->GetWidth(), packedTexture->GetHeight(), packedTexture->GetTextureFormat(),packedTexture->IsHalfFloat());
						
						GLTextureExport(packedTexture->GetID(), info, graphics::PBRTextureType::MNSY_TEXTURE_NONE);						

						delete packedTexture;
						packedTexture = nullptr;
					}

				}
			}
		}

		return true;
	}

	void ExportManager::GLTextureExport(const int glTextureID, TextureExportInfo& exportInfo, graphics::PBRTextureType PBRTypeHint) {

		graphics::TextureFormat format = exportInfo.textureFormat;

		graphics::ImageFileFormat fileFormat = graphics::TexUtil::get_imageFileFormat_from_fileExtentionString(exportInfo.path.extension().generic_string());

		if (fileFormat == graphics::ImageFileFormat::MNSY_FILE_FORMAT_NONE) {
			MNEMOSY_ERROR("ExportManager::GLTextureExport: file format {} is not supported",exportInfo.path.extension().generic_string());
			return;
		}


		// todo this should not be handled here but rather by the specific writer
		if(format == graphics::MNSY_RG8 ||format == graphics::MNSY_RG16 ||format == graphics::MNSY_RG32 ){
			MNEMOSY_ERROR("ExportManager::GLTextureExport: Export of dual channel textures is not supported");
			return;
		}

		unsigned int width = exportInfo.width;
		unsigned int height = exportInfo.height;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glTextureID);


		// ensure byte alignment
		unsigned int restWidth = width % 4;
		unsigned int padding = 4;
		if (restWidth != 0) {
			padding = 1;
		}

		glPixelStorei(GL_PACK_ALIGNMENT, padding);



		uint8_t numChannels;
		uint8_t bitsPerChannel;
		uint8_t bytesPerPixel;
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
		size_t bufferSize = width * height * bytesPerPixel;
		void* pixelBuffer =  malloc(bufferSize);

		size_t glHalfFloatType = GL_UNSIGNED_SHORT;
		if (isHalfFloat) {
			glHalfFloatType = GL_HALF_FLOAT;
		}


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

		graphics::PictureInfo info;
		info.width = width;
		info.height = height;
		info.textureFormat = format;
		info.isHalfFloat = isHalfFloat;
		info.pixels = pixelBuffer;


		std::string exportFormatTxt = graphics::TexUtil::get_string_from_textureFormat(format);
		
		std::string exportPath = exportInfo.path.generic_string();

		graphics::PictureError errorCheck;
		graphics::Picture::WritePicture(errorCheck, exportPath.c_str(), info,true, PBRTypeHint);
		if (!errorCheck.wasSuccessfull) {
			MNEMOSY_ERROR("An error occured while exporting. Format: {} {}x{}  to: {} \n Error Message: {}", exportFormatTxt, width, height, exportPath, errorCheck.what);
		}
		
		if (pixelBuffer) {
			free(pixelBuffer);
		}

		MNEMOSY_INFO("Exported Texture: {}, {}x{} {}", exportPath, width, height, exportFormatTxt);
	}



} // !mnemosy::systems
