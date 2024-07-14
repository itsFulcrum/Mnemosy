#ifndef EXPORT_MANAGER_H
#define EXPORT_MANAGER_H


#include <vector>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;


namespace mnemosy::graphics
{
	class Material;
	class Texture;
	enum ExportImageFormat;
	enum NormalMapFormat;
}

namespace mnemosy::systems
{


	class ExportManager
	{
	public:
		ExportManager();
		~ExportManager();


		bool ExportMaterialTextures(fs::path& exportPath, fs::path& materialFolderPath,graphics::Material& material);



		void SetExportImageFormatInt(int format);
		void SetExportImageFormat(graphics::ExportImageFormat format) { m_exportImageFormat = format; }
		const int GetExportImageFormatInt() { return (const int)m_exportImageFormat; }

		void SetNormalMapExportFormatInt(int format);
		void SetNormalMapExportFormat(graphics::NormalMapFormat format) { m_exportNormalFormat = format; }
		const int GetNormalMapExportFormatInt() { return (const int)m_exportNormalFormat; }
		void ExportMaterialTexturePngOrTif(fs::path& exportPath,graphics::Texture& texture,bool singleChannelTexture,bool bits16);
		void ExportGlTexturePngOrTiff(fs::path& exportPath, int glTextureId, int width, int height);


		std::vector<std::string>& GetLastExportedFilePaths();
	private:
		void ExportAsKtx2(fs::path& exportPath, fs::path& materialFolderPath, graphics::Material& material);


		std::vector<std::string> m_lastExportedFilePaths;

		std::string GetExportNormalFormatString();
		std::string GetExportImageFormatString();

		graphics::ExportImageFormat m_exportImageFormat;
		graphics::NormalMapFormat m_exportNormalFormat;

	};


}
#endif // !EXPORT_MANAGER_H
