#ifndef EXPORT_MANAGER_H
#define EXPORT_MANAGER_H

#include "Include/Graphics/TextureDefinitions.h"


#include <vector>
#include <string>
#include <filesystem>
//namespace fs = std::filesystem;

namespace mnemosy::systems {
	enum LibEntryType;
	struct LibEntry;
	struct FolderNode;
}

namespace mnemosy::graphics
{
	class UnlitMaterial;
	class PbrMaterial;
	class Skybox;
	class Texture;
	enum NormalMapFormat;
}

namespace mnemosy::systems
{

	struct TextureExportInfo {
	public:
		
		TextureExportInfo() = default;

		TextureExportInfo(std::filesystem::path& _exportPath, uint16_t _width, uint16_t _height, graphics::TextureFormat _textureFormat,bool _isHalfFloat)
			: path{ _exportPath }
			, width{_width}
			, height{_height}
			, textureFormat{_textureFormat}
			, isHalfFloat{_isHalfFloat}
		{}

		TextureExportInfo(std::filesystem::path& _exportPath, uint16_t _width, uint16_t _height, graphics::TextureFormat _textureFormat, bool _isHalfFloat, bool _converExrAndHdrToLinear)
			: path{ _exportPath }
			, width{ _width }
			, height{ _height }
			, textureFormat{ _textureFormat }
			, isHalfFloat{ _isHalfFloat }
			, converExrAndHdrToLinear{_converExrAndHdrToLinear}
		{}


		std::filesystem::path& path;
		uint16_t width;
		uint16_t height;
		graphics::TextureFormat textureFormat;
		bool isHalfFloat; // half float here deterimines not the source data but in what data we want to export, important for .exr images
		bool converExrAndHdrToLinear = true; // hidden flag but usefull for skybox images
	};


	class ExportManager
	{
	public:
		ExportManager() = default;
		~ExportManager() = default;

		void Init();
		void Shutdown();

		bool PbrMat_ExportTextures(std::filesystem::path& exportFolderPath, systems::LibEntry* libEntry, graphics::PbrMaterial& pbrMat, std::vector<bool>& exportTypesOrdered, bool exportChannelPacked);
		
		void UnlitMat_ExportTextures(std::filesystem::path& exportFolderPath, systems::LibEntry* libEntry , graphics::UnlitMaterial& unlitMat);
		
		void SkyboxMat_ExportTextures(std::filesystem::path& exportFolderPath, systems::LibEntry* libEntry, graphics::Skybox& skyboxMat);


		void GLTextureExport(const int glTextureID, TextureExportInfo& exportInfo);
		

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
