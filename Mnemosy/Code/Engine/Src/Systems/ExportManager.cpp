#include "Include/Systems/ExportManager.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/TextureGenerationManager.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Material.h"

#include "Include/Graphics/Utils/KtxImage.h"
#include "Include/Graphics/Utils/Picture.h"

#include <glad/glad.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/logger.hpp>

namespace mnemosy::systems
{
	// public methods

	ExportManager::ExportManager()
		: m_exportImageFormat(graphics::MNSY_PNG)
		, m_exportNormalFormat(graphics::MNSY_NORMAL_FORMAT_OPENGL)
		, m_exportRoughnessAsSmoothness(false)
	{

		// shut up openCV logging
		//cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);

	}

	ExportManager::~ExportManager() {

	}


	// Export selected textures of a material. Which textures to export should be specified in the std::vector<bool> exportTypesOrdered which need an entry for each texture type in the same order as the enum types defined in PBRTextureType in TextureDefinitions.h
	bool ExportManager::ExportMaterialTextures(std::filesystem::path& exportPath, std::filesystem::path& materialFolderPath, graphics::Material& material, std::vector<bool>& exportTypesOrdered, bool exportChannelPacked) {

		namespace fs = std::filesystem;

		MNEMOSY_INFO("Exporting Material: {}, as {} using {} normal map format \nExport Path: {}", material.Name, GetExportImageFormatString(), GetExportNormalFormatString(), exportPath.generic_string());

		std::string fileExtention = ".png";
		if (m_exportImageFormat == graphics::MNSY_TIF) {
			fileExtention = ".tif";
		}

		// Export Albedo
		if (exportTypesOrdered[0]) {

			if (material.isAlbedoAssigned()) {

				fs::path to = exportPath / fs::path(material.Name + "_albedo_sRGB" + fileExtention);
				graphics::Texture& tex = material.GetAlbedoTexture();

				TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_RGBA8);
				
				info.imageFileType = (graphics::ImageFileType)m_exportImageFormat;
				ExportGlTexture_PngOrTiff(tex.GetID(), info);
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

					TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_R16);
					
					info.imageFileType = (graphics::ImageFileType)m_exportImageFormat;
					ExportGlTexture_PngOrTiff(tex.GetID(), info);
				}
			}
		}

		// Export Metallic
		if (exportTypesOrdered[2]) {

			if (material.isMetallicAssigned()) {
				fs::path to = exportPath / fs::path(material.Name + "_metallic_raw" + fileExtention);
				graphics::Texture& tex = material.GetMetallicTexture();

				TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_R16);
				info.imageFileType = (graphics::ImageFileType)m_exportImageFormat;

				ExportGlTexture_PngOrTiff(tex.GetID(), info);
			}
		}

		// Export Normal
		if (exportTypesOrdered[3]) {
			if (material.isNormalAssigned()) {

				if (m_exportNormalFormat == graphics::MNSY_NORMAL_FORMAT_OPENGL) {
					fs::path to = exportPath / fs::path(material.Name + "_normal_gl_raw" + fileExtention);
					graphics::Texture& tex = material.GetNormalTexture();

					TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_RGB16);
					info.imageFileType = (graphics::ImageFileType)m_exportImageFormat;
					ExportGlTexture_PngOrTiff(tex.GetID(), info);
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

				TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_R16);
				info.imageFileType = (graphics::ImageFileType)m_exportImageFormat;
				ExportGlTexture_PngOrTiff(tex.GetID(), info);
			}
		}

		// Export Emissive 
		if (exportTypesOrdered[5]) {
			if (material.isEmissiveAssigned()) {

				fs::path to = exportPath / fs::path(material.Name + "_emissive_sRGB" + fileExtention);
				graphics::Texture& tex = material.GetEmissiveTexture();

				TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_RGB8);
				info.imageFileType = (graphics::ImageFileType)m_exportImageFormat;
				ExportGlTexture_PngOrTiff(tex.GetID(), info);
			}
		}

		// Export Height
		if (exportTypesOrdered[6]) {
			if (material.isHeightAssigned()) {
				fs::path to = exportPath / fs::path(material.Name + "_height_raw" + fileExtention);
				graphics::Texture& tex = material.GetHeightTexture();

				TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_R16);
				info.imageFileType = (graphics::ImageFileType)m_exportImageFormat;
				ExportGlTexture_PngOrTiff(tex.GetID(), info);
			}
		}

		// Export Opacity
		if (exportTypesOrdered[7]) {
			if (material.isOpacityAssigned()) {
				fs::path to = exportPath / fs::path(material.Name + "_opacity_raw" + fileExtention);
				graphics::Texture& tex = material.GetOpacityTexture();

				TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_R16);
				info.imageFileType = (graphics::ImageFileType)m_exportImageFormat;
				ExportGlTexture_PngOrTiff(tex.GetID(), info);
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
					if (m_exportImageFormat == graphics::MNSY_TIF) {								
						
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

						graphics::Texture* packedTexture = new graphics::Texture();

						packedTexture->GenerateFromFile(pathOnDisk.generic_string().c_str(), true, false,graphics::MNSY_TEXTURE_NONE);


						graphics::TextureFormat texFormat = graphics::MNSY_RGB16;

						if (packedTexture->GetChannelsAmount() == 4) {
							texFormat = graphics::MNSY_RGBA16;
						}

						TextureExportInfo info = TextureExportInfo(to.generic_string(), packedTexture->GetWidth(), packedTexture->GetHeight(), texFormat);
						info.imageFileType = (graphics::ImageFileType)m_exportImageFormat;
						ExportGlTexture_PngOrTiff(packedTexture->GetID(), info);

						delete packedTexture;
						packedTexture = nullptr;
					}

				}
			}
		}

		return true;
	}

	// private methods

	std::string ExportManager::GetExportImageFormatString() {

		const char* exportFormats[] = { "tiff", "png"}; // they need to be ordered the same as in ExportManager ExportImageFormats

		return exportFormats[m_exportImageFormat];
	}

	void ExportManager::GLTextureExport(const int glTextureID, TextureExportInfo& exportInfo) {

		// in development Not using this for now but I'd like to in the future if my picture wrapper interface is propperly implemented




		// update this
		if (exportInfo.imageFileType != graphics::ImageFileType::MNSY_FILE_TYPE_TIF) {
			MNEMOSY_ERROR("Only Tiff Is fully supported for export atm")
		}

		graphics::TextureFormat format = exportInfo.textureFormat;


		if(format == graphics::MNSY_RG8 ||format == graphics::MNSY_RG8 ||format == graphics::MNSY_RG8 ){
			MNEMOSY_ERROR("ExportManager::GLTextureExport: Export of dual channel textures is not supported");
			return;
		}

		unsigned int width = exportInfo.width;
		unsigned int height = exportInfo.height;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glTextureID);


		// row byte alingment for textures that have width not divisible by 4
		int rest = width % 2;

		if (rest == 0) {
			glPixelStorei(GL_PACK_ALIGNMENT, 4);
		}
		else if (rest == 1) {
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
		}

		MNEMOSY_WARN("USING GL EXPORT TEXTURE");

		
		// amount of color channels
		uint8_t channels = (uint8_t)format % 4;
		if (channels == 0) {
			channels = 4;
		}

		// asume 8 bits per pixel
		uint8_t bitsPerChannel = 8;
		size_t bytesPerPixel = channels * sizeof(uint8_t);

		if(format == graphics::MNSY_R16 || format == graphics::MNSY_RG16 || format ==  graphics::MNSY_RGB16 || format == graphics::MNSY_RGBA16){
			bitsPerChannel = 16;
			bytesPerPixel = channels * sizeof(uint16_t);
		}
		else if(format == graphics::MNSY_R32 || format == graphics::MNSY_RG32 || format == graphics::MNSY_RGB32 || format == graphics::MNSY_RGBA32){
			bitsPerChannel = 32;
			bytesPerPixel = channels * sizeof(uint32_t);
		}

		
		// buffer for pixel data
		size_t bufferSize = width * height * bytesPerPixel;
		void* pixelBuffer =  malloc(bufferSize);

		graphics::PictureInfo info;
		info.width = width;
		info.height = height;
		info.textureFormat = format;
		info.pixels = pixelBuffer;

		switch (format)
		{
		// 8 bits per pixel
		case (graphics::MNSY_R8):	// not tested
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, pixelBuffer);
			break;
		// case (graphics::MNSY_RG8): // not supported for most file types
		// 	glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_UNSIGNED_BYTE, pixelBuffer);
		// 	break;
		case (graphics::MNSY_RGB8): // works
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);
			break;
		case (graphics::MNSY_RGBA8): // works
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);
			break;

		// 16 bits per pixel
		case (graphics::MNSY_R16): // not tested
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, pixelBuffer);
			break;
		// case (graphics::MNSY_RG16): // not supported for most file types
		// 	glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_UNSIGNED_SHORT, pixelBuffer);
		// 	break;
		case (graphics::MNSY_RGB16):	// not tested
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_SHORT, pixelBuffer);
			break;
		case (graphics::MNSY_RGBA16):	// not tested
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_SHORT, pixelBuffer);
			break;

		// 32 bits per pixel
		case (graphics::MNSY_R32): // not tested
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pixelBuffer);
			break;
		// case (graphics::MNSY_RG32): // not supported for most file types
		// 	glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, pixelBuffer);
		// 	break;
		case (graphics::MNSY_RGB32): // not tested
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, pixelBuffer);
			break;
		case (graphics::MNSY_RGBA32): // not tested
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixelBuffer);
			break;
		}

		// used only for printing log messages
		std::string exportFormatTxt = graphics::TextureDefinitions::GetTextureFormatAsString(format);


		// write thie image to file
		graphics::PictureErrorCode errorCode = graphics::PictureErrorCode::MNSY_PICTURE_ERROR_NONE;
		
		if(exportInfo.imageFileType == graphics::ImageFileType::MNSY_FILE_TYPE_TIF){

			graphics::Picture::WriteTiff(errorCode, exportInfo.path.c_str(), info,true);
			if(errorCode != graphics::PictureErrorCode::MNSY_PICTURE_ERROR_NONE){
				MNEMOSY_ERROR("An error occured while exporting. Format: {} {}x{}  to: {} \n Error Message: {}",exportFormatTxt, width, height, exportInfo.path, graphics::Picture::ErrorStringGet(errorCode));
			}
		}
		// Picture inteface does not free the buffer itself
		free(pixelBuffer);

		if(errorCode == graphics::PictureErrorCode::MNSY_PICTURE_ERROR_NONE){
			MNEMOSY_INFO("Exported Texture: Format: {} {}x{}  to: {}", exportFormatTxt, width, height, exportInfo.path);
		}
	}

	void ExportManager::ExportGlTexture_PngOrTiff(const int glTextureId, TextureExportInfo& exportInfo) {

		unsigned int width = exportInfo.width;
		unsigned int height = exportInfo.height;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glTextureId);


		// row byte alingment for textures that have width not divisible by 4
		int rest = width % 2;

		if (rest == 0) {
			glPixelStorei(GL_PACK_ALIGNMENT, 4);
		}
		else if (rest == 1) {
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
		}



		cv::Mat img;

		graphics::TextureFormat format = exportInfo.textureFormat;
		std::string exportFormatTxt  = "";

		if (format == graphics::MNSY_R8 || format == graphics::MNSY_RG8 || format == graphics::MNSY_RGB8 || format == graphics::MNSY_RGBA8) {

			unsigned int channels = (unsigned int)format;
			MNEMOSY_ASSERT(channels != 2, "ExportManager::ExportGlTexturePngOrTiff: Export of dual channel textures is not supoorted for png or tiff images");


			void* gl_texture_bytes = malloc(sizeof(uint8_t) * width * height * channels);

			if (channels == 1) { // Not Tested

				glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, gl_texture_bytes);
				img = cv::Mat(height, width, CV_8UC1, gl_texture_bytes);
				exportFormatTxt = "R8";
			}
			else if (channels == 3) { // Tested And Works

				glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, gl_texture_bytes);
				img = cv::Mat(height, width, CV_8UC3, gl_texture_bytes);
				exportFormatTxt = "RGB8";
			}
			else if (channels == 4) { // Tested And Works

				glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, gl_texture_bytes);
				img = cv::Mat(height, width, CV_8UC4, gl_texture_bytes);
				exportFormatTxt = "RGBA8";
			}


			cv::flip(img, img, 0);
			cv::imwrite(exportInfo.path, img);
			img.release();

			free(gl_texture_bytes);

		}
		else if (format == graphics::MNSY_R16 || format == graphics::MNSY_RG16 || format == graphics::MNSY_RGB16 || format == graphics::MNSY_RGBA16) {

			unsigned int channels = (unsigned int)format - 4;
			MNEMOSY_ASSERT(channels != 2, "ExportManager::ExportGlTexturePngOrTiff: Export of dual channel textures is not supoorted for png or tiff images");

			void* gl_texture_bytes = malloc(sizeof(uint16_t) * width * height * channels);

			if (channels == 1) { // Tested and Works

				glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, gl_texture_bytes);

				img = cv::Mat(height, width, CV_16UC1, gl_texture_bytes);
				exportFormatTxt = "R16";
			}
			else if (channels == 3) { // Tested and works

				
				glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_SHORT, gl_texture_bytes);
				img = cv::Mat(height, width, CV_16UC3, gl_texture_bytes);

				exportFormatTxt = "RGB16";
			}
			else if (channels == 4) { // Not Tested

				glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_SHORT, gl_texture_bytes);
				img = cv::Mat(height, width, CV_16UC4, gl_texture_bytes);
				exportFormatTxt = "RGBA16";
			}
			

			cv::flip(img, img, 0);
			cv::imwrite(exportInfo.path, img);
			img.release();


			free(gl_texture_bytes);
		}
		// Not Tested
		else if (format == graphics::MNSY_R32 || format == graphics::MNSY_RG32 || format == graphics::MNSY_RGB32 || format == graphics::MNSY_RGBA32) {

			unsigned int channels = (unsigned int)format - 8;

			MNEMOSY_ASSERT(channels != 2, "ExportManager::ExportGlTexturePngOrTiff: Export of dual channel textures is not supoorted for png or tiff images");
			
			void* gl_texture_bytes = malloc(sizeof(uint32_t) * width * height * channels);


			if (channels == 1) { // Not Tested
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, gl_texture_bytes);
				img = cv::Mat(height, width, CV_32FC1, gl_texture_bytes);
				exportFormatTxt = "R32";
			}
			else if (channels == 3) {// Not Tested

				glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_FLOAT, gl_texture_bytes);
				img = cv::Mat(height, width, CV_32FC3, gl_texture_bytes);
				exportFormatTxt = "RGB32";
			}
			else if (channels == 4) {// Not Tested

				glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_FLOAT, gl_texture_bytes);
				img = cv::Mat(height, width, CV_32FC4, gl_texture_bytes);
				exportFormatTxt = "RGBA32";
			}
			

			cv::flip(img, img, 0);
			cv::imwrite(exportInfo.path, img);
			img.release();

			free(gl_texture_bytes);
		}


		MNEMOSY_INFO("Exported Texture: Format: {} {}x{}  to: {}", exportFormatTxt, width, height, exportInfo.path);
	}

	void ExportManager::ExportAsKtx2(std::filesystem::path& exportPath, std::filesystem::path& materialFolderPath, graphics::Material& material) {

		namespace fs = std::filesystem;
		// For ktx2 we can just copy some of the files but we have to check
		// if we need to convert the normal format 
		// and we need to export albedo and emission as srgb explicity..

		// Special Cases
		// we store color images internally in linear format but it is convention to have them in sRGB so we need to convert them at export time.
		if (material.isAlbedoAssigned()) {

			fs::path to = exportPath / fs::path(material.Name + "_albedo_sRGB.ktx2");
			graphics::KtxImage img;
			graphics::Texture& albedo = material.GetAlbedoTexture();
			img.ExportGlTexture(to.generic_string().c_str(), albedo.GetID(), albedo.GetChannelsAmount(), albedo.GetWidth(), albedo.GetHeight(), graphics::MNSY_COLOR_SRGB, true);
			
			MNEMOSY_INFO("Exported: {}", to.generic_string());
		}

		if (material.isEmissiveAssigned()) {

			fs::path to = exportPath / fs::path(material.Name + "_emissive_sRGB.ktx2");
			graphics::KtxImage img;
			graphics::Texture& emissive = material.GetEmissiveTexture();
			img.ExportGlTexture(to.generic_string().c_str(), emissive.GetID(), emissive.GetChannelsAmount(), emissive.GetWidth(), emissive.GetHeight(), graphics::MNSY_COLOR_SRGB, true);
			
			MNEMOSY_INFO("Exported: {}", to.generic_string());
		}

		if (material.isNormalAssigned()) {

			// we store in opengl format internally so we can just copy the file
			if (m_exportNormalFormat == graphics::MNSY_NORMAL_FORMAT_OPENGL) {
				try {
					fs::path from = materialFolderPath / fs::path(material.Name + "_normal.ktx2");
					fs::path to = exportPath / fs::path(material.Name + "_normal_gl_raw.ktx2");
					fs::copy_file(from, to, fs::copy_options::overwrite_existing);
					
					MNEMOSY_INFO("Exported: {}", to.generic_string());
				}
				catch (fs::filesystem_error error) {
					MNEMOSY_ERROR("ExprortManager::ExportMaterialTextures: System error copying file {}", error.what());
				}
			}
			// need to convert it to directX format
			else if (m_exportNormalFormat == graphics::MNSY_NORMAL_FORMAT_DIRECTX) {

				TextureGenerationManager& texGenerator = MnemosyEngine::GetInstance().GetTextureGenerationManager();
				fs::path to = exportPath / fs::path(material.Name + "_normal_dx_raw.ktx2");

				texGenerator.FlipNormalMap(to.generic_string().c_str(), material,true);
				
				MNEMOSY_INFO("Exported: {}", to.generic_string());
			}
		}

		// we can just copy
		if (material.isRoughnessAssigned()) {
			try {
				fs::path from = materialFolderPath / fs::path(material.Name + "_roughness.ktx2");
				fs::path to = exportPath / fs::path(material.Name + "_roughness_raw.ktx2");
				fs::copy_file(from, to, fs::copy_options::overwrite_existing);
				
				MNEMOSY_INFO("Exported: {}", to.generic_string());
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("ExprortManager::ExportMaterialTextures: System error copying file {}", error.what());
			}
		}
		if (material.isMetallicAssigned()) {
			try {
				fs::path from = materialFolderPath / fs::path(material.Name + "_metallic.ktx2");
				fs::path to = exportPath / fs::path(material.Name + "_metallic_raw.ktx2");
				fs::copy_file(from, to, fs::copy_options::overwrite_existing);
				
				MNEMOSY_INFO("Exported: {}", to.generic_string());
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("ExprortManager::ExportMaterialTextures: System error copying file {}", error.what());
			}
		}
		if (material.isAoAssigned()) {
			try {
				fs::path from = materialFolderPath / fs::path(material.Name + "_ambientOcclusion.ktx2");
				fs::path to = exportPath / fs::path(material.Name + "_ambientOcclusion_raw.ktx2");
				fs::copy_file(from, to, fs::copy_options::overwrite_existing);
				
				MNEMOSY_INFO("Exported: {}", to.generic_string());
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("ExprortManager::ExportMaterialTextures: System error copying file {}", error.what());
			}
		}

	}

	std::string ExportManager::GetExportNormalFormatString() {
		const char* normalMapFormats[] = { "OpenGl", "DirectX" }; // they need to be ordered the same as in material NormalMapFormat Enum
		return normalMapFormats[m_exportNormalFormat];
	}

} // !mnemosy::systems
