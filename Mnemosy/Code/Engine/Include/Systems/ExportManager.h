#ifndef EXPORT_MANAGER_H
#define EXPORT_MANAGER_H

#include "Include/Graphics/Material.h"


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
	enum ExportImageFormat
	{
		MNSY_KTX2 = 0,
		MNSY_PNG = 1,
		MNSY_TIF = 2
	};

	class ExportManager
	{
	public:
		ExportManager();
		~ExportManager();


		bool ExportMaterialTextures(fs::path& exportPath, fs::path& materialFolderPath,graphics::Material& material);



		void SetExportImageFormatInt(int format);
		void SetExportImageFormat(ExportImageFormat format) { m_exportImageFormat = format; }
		const int GetExportImageFormatInt() { return (const int)m_exportImageFormat; }

		void SetNormalMapExportFormatInt(int format);
		void SetNormalMapExportFormat(graphics::NormalMapFormat format) { m_exportNormalFormat = format; }
		const int GetNormalMapExportFormatInt() { return (const int)m_exportNormalFormat; }

	private:
		void ExportAsKtx2(fs::path& exportPath, fs::path& materialFolderPath, graphics::Material& material);
		void ExportMaterialTexturePngOrTif(fs::path& exportPath,graphics::Texture& texture,bool singleChannelTexture,bool bits16);

		std::string GetExportNormalFormatString();
		std::string GetExportImageFormatString();

		ExportImageFormat m_exportImageFormat = systems::MNSY_PNG;
		graphics::NormalMapFormat m_exportNormalFormat = graphics::MNSY_NORMAL_FORMAT_OPENGl;

	};


}
#endif // !EXPORT_MANAGER_H