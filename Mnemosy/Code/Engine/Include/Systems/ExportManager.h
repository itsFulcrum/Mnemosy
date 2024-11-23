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

		TextureExportInfo(std::filesystem::path _exportPath, uint16_t _width, uint16_t _height, graphics::TextureFormat _textureFormat,bool _isHalfFloat)
			: path{ _exportPath }
			, width{_width}
			, height{_height}
			, textureFormat{_textureFormat}
			, isHalfFloat{_isHalfFloat}
		{}

		std::filesystem::path path;
		uint16_t width;
		uint16_t height;
		graphics::TextureFormat textureFormat;
		bool isHalfFloat; // half float here deterimines not the source data but in what data we want to export, important for .exr images
	};


	class ExportManager
	{
	public:
		ExportManager() = default;
		~ExportManager() = default;

		void Init();
		void Shutdown();

		bool ExportMaterialTextures(std::filesystem::path& exportPath, std::filesystem::path& materialFolderPath, graphics::Material& material, std::vector<bool>& exportTypesOrdered, bool exportChannelPacked);

		void GLTextureExport(const int glTextureID, TextureExportInfo& exportInfo, graphics::PBRTextureType PBRTypeHint);
		

		// Export settings

		const graphics::ImageFileFormat GetExportImageFormat() { return m_exportFileFormat; }
		void SetExportImageFormat(graphics::ImageFileFormat format) { m_exportFileFormat = format; }

		graphics::NormalMapFormat GetNormalMapExportFormat() { return m_exportNormalFormat; }
		void SetNormalMapExportFormat(graphics::NormalMapFormat format) { m_exportNormalFormat = format; }


		void SetExportRoughnessAsSmoothness(bool exportRoughAsSmooth) { m_exportRoughnessAsSmoothness = exportRoughAsSmooth; }
		bool GetExportRoughnessAsSmoothness() { return m_exportRoughnessAsSmoothness; }

	private:
		
		graphics::ImageFileFormat m_exportFileFormat;
		graphics::NormalMapFormat m_exportNormalFormat;
		bool m_exportRoughnessAsSmoothness;
	};


}
#endif // !EXPORT_MANAGER_H
