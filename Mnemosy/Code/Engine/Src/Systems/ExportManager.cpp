#include "Include/Systems/ExportManager.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Systems/TextureGenerationManager.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Material.h"

#include "Include/Graphics/Utils/KtxImage.h"

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
		, m_exportNormalFormat(graphics::MNSY_NORMAL_FORMAT_OPENGl)
		, m_exportRoughnessAsSmoothness(false)
	{

		/*m_exportImageFormat = graphics::MNSY_PNG;
		m_exportNormalFormat = graphics::MNSY_NORMAL_FORMAT_OPENGl;*/

		// shut up openCV logging
		//cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
	}

	ExportManager::~ExportManager() {

		if (!m_lastExportedFilePaths.empty())
			m_lastExportedFilePaths.clear();

	}

	bool ExportManager::ExportMaterialTextures(fs::path& exportPath, fs::path& materialFolderPath, graphics::Material& material) {

		MNEMOSY_INFO("Exporting Material: {}, as {} using {} normal map format \nExport Path: {}", material.Name, GetExportImageFormatString(), GetExportNormalFormatString(), exportPath.generic_string());

		// store the filepath of every file we export in this vector so we can use it when dragging files out of mnemosy
		if (!m_lastExportedFilePaths.empty())
			m_lastExportedFilePaths.clear();


		std::string fileExtention = ".png";
		if (m_exportImageFormat == graphics::MNSY_TIF) {
			fileExtention = ".tif";
		}

		if (material.isAlbedoAssigned()) {

			fs::path to = exportPath / fs::path(material.Name + "_albedo_sRGB" + fileExtention);
			graphics::Texture& tex = material.GetAlbedoTexture();

			TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_RGBA8);
			ExportGlTexture_PngOrTiff(tex.GetID(),info);

			m_lastExportedFilePaths.push_back(to.generic_string());
		}

		if (material.isEmissiveAssigned()) {

			fs::path to = exportPath / fs::path(material.Name + "_emissive_sRGB" + fileExtention);
			graphics::Texture& tex = material.GetEmissiveTexture();

			TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_RGB8);
			ExportGlTexture_PngOrTiff(tex.GetID(), info);

			m_lastExportedFilePaths.push_back(to.generic_string());
		}
		if (material.isNormalAssigned()) {

			if (m_exportNormalFormat == graphics::MNSY_NORMAL_FORMAT_OPENGl) {
				fs::path to = exportPath / fs::path(material.Name + "_normal_gl_raw" + fileExtention);
				graphics::Texture& tex = material.GetNormalTexture();
				
				TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_RGB16);
				ExportGlTexture_PngOrTiff(tex.GetID(), info);

				m_lastExportedFilePaths.push_back(to.generic_string());
			}
			else if (m_exportNormalFormat == graphics::MNSY_NORMAL_FORMAT_DIRECTX) {
			// Convert and then export
				TextureGenerationManager& texGenerator = MnemosyEngine::GetInstance().GetTextureGenerationManager();			
				
				fs::path to = exportPath / fs::path(material.Name + "_normal_dx_raw" + fileExtention);
				texGenerator.FlipNormalMap(to.generic_string().c_str(), material, true);

				m_lastExportedFilePaths.push_back(to.generic_string());
			}
		}
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
				ExportGlTexture_PngOrTiff(tex.GetID(), info);



				m_lastExportedFilePaths.push_back(to.generic_string());

			}




		}
		if (material.isMetallicAssigned()) {
			fs::path to = exportPath / fs::path(material.Name + "_metallic_raw" + fileExtention);
			graphics::Texture& tex = material.GetMetallicTexture();

			TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_R16);
			ExportGlTexture_PngOrTiff(tex.GetID(), info);

			m_lastExportedFilePaths.push_back(to.generic_string());
		}
		if (material.isAoAssigned()) {
			fs::path to = exportPath / fs::path(material.Name + "_ambientOcclusion_raw" + fileExtention);
			graphics::Texture& tex = material.GetAOTexture();
			
			TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_R16);
			ExportGlTexture_PngOrTiff(tex.GetID(), info);
			
			m_lastExportedFilePaths.push_back(to.generic_string());
		}


		if (material.isHeightAssigned()) {
			fs::path to = exportPath / fs::path(material.Name + "_height_raw" + fileExtention);
			graphics::Texture& tex = material.GetHeightTexture();

			TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_R16);
			ExportGlTexture_PngOrTiff(tex.GetID(), info);

			m_lastExportedFilePaths.push_back(to.generic_string());
		}

		if (material.isOpacityAssigned()) {
			fs::path to = exportPath / fs::path(material.Name + "_opacity_raw" + fileExtention);
			graphics::Texture& tex = material.GetOpacityTexture();

			TextureExportInfo info = TextureExportInfo(to.generic_string(), tex.GetWidth(), tex.GetHeight(), graphics::MNSY_R16);
			ExportGlTexture_PngOrTiff(tex.GetID(), info);

			m_lastExportedFilePaths.push_back(to.generic_string());
		}

		return true;
	}

	void ExportManager::SetExportImageFormatInt(int format) {
		//MNSY_KTX2 = 0;
		//MNSY_PNG = 1;
		//MNSY_TIF = 2;
		


		if (format == 0) {
			m_exportImageFormat = graphics::MNSY_TIF;
		}
		else if (format == 1) {
			m_exportImageFormat = graphics::MNSY_PNG;
		}
		//else if (format == 2) {
		//	m_exportImageFormat = MNSY_TIF;
		//}
	}

	

	void ExportManager::SetNormalMapExportFormatInt(int format) {
		if (format == 0) {
			m_exportNormalFormat = graphics::MNSY_NORMAL_FORMAT_OPENGl;
		}
		else if (format == 1) {
			m_exportNormalFormat = graphics::MNSY_NORMAL_FORMAT_DIRECTX;
		}
	}

	// private methods

	std::string ExportManager::GetExportImageFormatString() {

		const char* exportFormats[] = { "tiff", "png"}; // they need to be ordered the same as in ExportManager ExportImageFormats

		return exportFormats[m_exportImageFormat];
	}

	void ExportManager::ExportGlTexture_PngOrTiff(const int glTextureId, TextureExportInfo& exportInfo) {

		unsigned int width = exportInfo.width;
		unsigned int height = exportInfo.height;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glTextureId);

		cv::Mat img;

		graphics::TextureFormat format = exportInfo.textureFormat;
		std::string exportFormatTxt  = "";

		if (format == graphics::MNSY_R8 || format == graphics::MNSY_RG8 || format == graphics::MNSY_RGB8 || format == graphics::MNSY_RGBA8) {

			unsigned int channels = (unsigned int)format;

			char* gl_texture_bytes = (char*)malloc(sizeof(char) * width * height * channels);

			if (channels == 1) { // Not Tested
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, gl_texture_bytes);
				img = cv::Mat(height, width, CV_8UC1, gl_texture_bytes);
				exportFormatTxt = "R8";
			}
			else if (channels == 2) { // Not Tested
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_UNSIGNED_BYTE, gl_texture_bytes);
				img = cv::Mat(height, width, CV_8UC2, gl_texture_bytes);
				exportFormatTxt = "RG8";
			}
			else if (channels == 3) { // Tested And Works

				glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, gl_texture_bytes);
				img = cv::Mat(height, width, CV_8UC3, gl_texture_bytes);
				exportFormatTxt = "RGB8";
			}
			else if (channels == 4) { // Not Tested

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

			unsigned short* gl_texture_bytes = (unsigned short*)malloc(sizeof(unsigned short) * width * height * channels);


			if (channels == 1) { // Tested and Works
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, gl_texture_bytes);

				img = cv::Mat(height, width, CV_16UC1, gl_texture_bytes);
				exportFormatTxt = "R16";
			}
			else if (channels == 2) { // Not Tested

				glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_UNSIGNED_SHORT, gl_texture_bytes);
				img = cv::Mat(height, width, CV_16UC2, gl_texture_bytes);

				exportFormatTxt = "RG16";
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

			float* gl_texture_bytes = (float*)malloc(sizeof(float) * width * height * channels);


			if (channels == 1) { // Not Tested
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, gl_texture_bytes);
				img = cv::Mat(height, width, CV_32FC1, gl_texture_bytes);
				exportFormatTxt = "R32";
			}
			else if (channels == 2) {// Not Tested

				glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, gl_texture_bytes);
				img = cv::Mat(height, width, CV_32FC2, gl_texture_bytes);
				exportFormatTxt = "RG32";
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

	std::vector<std::string>& ExportManager::GetLastExportedFilePaths() {

		if (!m_lastExportedFilePaths.empty()) {
			return m_lastExportedFilePaths;
		}

		MNEMOSY_WARN("ExportManager::GetLastExportedFilePaths: Vector of filepaths is empty.");

		return m_lastExportedFilePaths;
		// TODO: insert return statement here
	}

	void ExportManager::ExportAsKtx2(fs::path& exportPath, fs::path& materialFolderPath, graphics::Material& material) {

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
			m_lastExportedFilePaths.push_back(to.generic_string());
			MNEMOSY_INFO("Exported: {}", to.generic_string());
		}

		if (material.isEmissiveAssigned()) {

			fs::path to = exportPath / fs::path(material.Name + "_emissive_sRGB.ktx2");
			graphics::KtxImage img;
			graphics::Texture& emissive = material.GetEmissiveTexture();
			img.ExportGlTexture(to.generic_string().c_str(), emissive.GetID(), emissive.GetChannelsAmount(), emissive.GetWidth(), emissive.GetHeight(), graphics::MNSY_COLOR_SRGB, true);
			m_lastExportedFilePaths.push_back(to.generic_string());
			MNEMOSY_INFO("Exported: {}", to.generic_string());
		}

		if (material.isNormalAssigned()) {

			// we store in opengl format internally so we can just copy the file
			if (m_exportNormalFormat == graphics::MNSY_NORMAL_FORMAT_OPENGl) {
				try {
					fs::path from = materialFolderPath / fs::path(material.Name + "_normal.ktx2");
					fs::path to = exportPath / fs::path(material.Name + "_normal_gl_raw.ktx2");
					fs::copy_file(from, to, fs::copy_options::overwrite_existing);
					m_lastExportedFilePaths.push_back(to.generic_string());
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
				m_lastExportedFilePaths.push_back(to.generic_string());
				MNEMOSY_INFO("Exported: {}", to.generic_string());
			}
		}

		// we can just copy
		if (material.isRoughnessAssigned()) {
			try {
				fs::path from = materialFolderPath / fs::path(material.Name + "_roughness.ktx2");
				fs::path to = exportPath / fs::path(material.Name + "_roughness_raw.ktx2");
				fs::copy_file(from, to, fs::copy_options::overwrite_existing);
				m_lastExportedFilePaths.push_back(to.generic_string());
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
				m_lastExportedFilePaths.push_back(to.generic_string());
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
				m_lastExportedFilePaths.push_back(to.generic_string());
				MNEMOSY_INFO("Exported: {}", to.generic_string());
			}
			catch (fs::filesystem_error error) {
				MNEMOSY_ERROR("ExprortManager::ExportMaterialTextures: System error copying file {}", error.what());
			}
		}

	}

	void ExportManager::ExportMaterialTexturePngOrTif_Depricated(fs::path& exportPath, graphics::Texture& texture, bool singleChannelTexture, bool bits16) {

		int width = texture.GetWidth();
		int height = texture.GetHeight();

		int channels = texture.GetChannelsAmount(); 
		if (singleChannelTexture)
			channels = 1;

		texture.BindToLocation(0);
		cv::Mat img;

		if (bits16) {
			unsigned short* gl_texture_bytes = (unsigned short*)malloc(sizeof(unsigned short) * width * height * channels);
			if (channels == 4) {

				glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_SHORT, gl_texture_bytes);
				img = cv::Mat(height, width, CV_16UC4, gl_texture_bytes);

			}
			else if (channels == 3) {

				glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_SHORT, gl_texture_bytes);
				img = cv::Mat(height, width, CV_16UC3, gl_texture_bytes);
			}
			else if (channels == 1) {
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, gl_texture_bytes);
				img = cv::Mat(height, width, CV_16UC1, gl_texture_bytes);
			}

			cv::flip(img, img, 0);
			cv::imwrite(exportPath.generic_string().c_str(), img);
			img.release();

			free(gl_texture_bytes);
		}
		else {
			char* gl_texture_bytes = (char*)malloc(sizeof(char) * width * height * channels);
			if (channels == 4) {

				glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, gl_texture_bytes);
				img = cv::Mat(height, width, CV_8UC4, gl_texture_bytes);
			}
			else if (channels == 3) {

				glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, gl_texture_bytes);
				img = cv::Mat(height, width, CV_8UC3, gl_texture_bytes);
			}
			else if (channels == 1) {
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, gl_texture_bytes);
				img = cv::Mat(height, width, CV_8UC1, gl_texture_bytes);
			}


			cv::flip(img, img, 0);
			cv::imwrite(exportPath.generic_string().c_str(), img);
			img.release();

			free(gl_texture_bytes);
		}	


		MNEMOSY_INFO("Exported: {}", exportPath.generic_string());
	}

	void ExportManager::ExportGlTexturePngOrTiff_Depricated(fs::path& exportPath, int glTextureId, int width, int height) {

		// TODO: Make this function work for all types of textures 
		// Only designed to be used with normal maps atm

		cv::Mat img;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glTextureId);
		
		unsigned short* gl_texture_bytes = (unsigned short*)malloc(sizeof(unsigned short) * width * height * 3);

		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_SHORT, gl_texture_bytes);
		img = cv::Mat(height, width, CV_16UC3, gl_texture_bytes);


		cv::flip(img, img, 0);
		cv::imwrite(exportPath.generic_string().c_str(), img);
		img.release();

		free(gl_texture_bytes);
	}





	std::string ExportManager::GetExportNormalFormatString()
	{
		const char* normalMapFormats[] = { "OpenGl", "DirectX" }; // they need to be ordered the same as in material NormalMapFormat Enum
		return normalMapFormats[m_exportNormalFormat];
	}




} // !mnemosy::systems
