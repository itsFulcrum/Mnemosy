#ifndef EXPORT_MANAGER_H
#define EXPORT_MANAGER_H

#include "Include/Graphics/TextureDefinitions.h"


#include <vector>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;



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


		bool ExportMaterialTextures(fs::path& exportPath, fs::path& materialFolderPath,graphics::Material& material, std::vector<bool>& exportTypesOrdered, bool exportChannelPacked);



		void SetExportImageFormatInt(int format);
		void SetExportImageFormat(graphics::ExportImageFormat format) { m_exportImageFormat = format; }
		const int GetExportImageFormatInt() { return (const int)m_exportImageFormat; }

		void SetNormalMapExportFormatInt(int format);
		void SetNormalMapExportFormat(graphics::NormalMapFormat format) { m_exportNormalFormat = format; }
		const int GetNormalMapExportFormatInt() { return (const int)m_exportNormalFormat; }
		
		void ExportMaterialTexturePngOrTif_Depricated(fs::path& exportPath,graphics::Texture& texture,bool singleChannelTexture,bool bits16);
		void ExportGlTexturePngOrTiff_Depricated(fs::path& exportPath, int glTextureId, int width, int height);


		void ExportGlTexture_PngOrTiff(const int glTextureId,TextureExportInfo& exportInfo);

		std::vector<std::string>& GetLastExportedFilePaths();


		void SetExportRoughnessAsSmoothness(bool exportRoughAsSmooth) { m_exportRoughnessAsSmoothness = exportRoughAsSmooth; }
		bool GetExportRoughnessAsSmoothness() { return m_exportRoughnessAsSmoothness; }
	private:
		void ExportAsKtx2(fs::path& exportPath, fs::path& materialFolderPath, graphics::Material& material);


		std::vector<std::string> m_lastExportedFilePaths;

		std::string GetExportNormalFormatString();
		std::string GetExportImageFormatString();

		graphics::ExportImageFormat m_exportImageFormat;
		graphics::NormalMapFormat m_exportNormalFormat;
		bool m_exportRoughnessAsSmoothness;
	};


}
#endif // !EXPORT_MANAGER_H
