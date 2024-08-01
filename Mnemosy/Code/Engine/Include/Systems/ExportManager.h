#ifndef EXPORT_MANAGER_H
#define EXPORT_MANAGER_H

#include "Include/Graphics/TextureDefinitions.h"


#include <vector>
#include <string>
#include <filesystem>
//namespace fs = std::filesystem;



namespace mnemosy::graphics
{
	class Material;
	class Texture;
	enum NormalMapFormat;
}

namespace mnemosy::systems
{

	struct TextureExportInfo {
	public:
		
		TextureExportInfo() = default;

		TextureExportInfo(std::string _exportPath, unsigned int _width, unsigned int _height, graphics::TextureFormat _textureFormat)
			: path{ _exportPath }
			, width{_width}
			, height{_height}
			, textureFormat{_textureFormat}
		{}

		std::string path;
		unsigned int width;
		unsigned int height;
		graphics::TextureFormat textureFormat;
	};


	class ExportManager
	{
	public:
		ExportManager();
		~ExportManager();


		bool ExportMaterialTextures(std::filesystem::path& exportPath, std::filesystem::path& materialFolderPath, graphics::Material& material, std::vector<bool>& exportTypesOrdered, bool exportChannelPacked);

		
		void SetExportImageFormat(graphics::ExportImageFormat format) { m_exportImageFormat = format; }
		const graphics::ExportImageFormat GetExportImageFormat() { return m_exportImageFormat; }





		//void SetNormalMapExportFormatInt(int format);
		void SetNormalMapExportFormat(graphics::NormalMapFormat format) { m_exportNormalFormat = format; }
		graphics::NormalMapFormat GetNormalMapExportFormat() { return m_exportNormalFormat; }


		void ExportGlTexture_PngOrTiff(const int glTextureId,TextureExportInfo& exportInfo);

		std::vector<std::string>& GetLastExportedFilePaths();


		void SetExportRoughnessAsSmoothness(bool exportRoughAsSmooth) { m_exportRoughnessAsSmoothness = exportRoughAsSmooth; }
		bool GetExportRoughnessAsSmoothness() { return m_exportRoughnessAsSmoothness; }
	private:
		void ExportAsKtx2(std::filesystem::path& exportPath, std::filesystem::path& materialFolderPath, graphics::Material& material);


		std::vector<std::string> m_lastExportedFilePaths;

		std::string GetExportNormalFormatString();
		std::string GetExportImageFormatString();

		graphics::ExportImageFormat m_exportImageFormat;
		graphics::NormalMapFormat m_exportNormalFormat;
		bool m_exportRoughnessAsSmoothness;
	};


}
#endif // !EXPORT_MANAGER_H
