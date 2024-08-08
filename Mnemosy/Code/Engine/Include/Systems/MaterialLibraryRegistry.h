#ifndef MATERIAL_LIBRARY_REGISTRY_H
#define MATERIAL_LIBRARY_REGISTRY_H

#include <vector>
#include <string>
#include <filesystem>

#include <nlohmann/json.hpp>

namespace mnemosy::systems {
	struct MaterialInfo;
	struct FolderNode;
	class FolderTree;
}
namespace mnemosy::core {
	class FileDirectories;
}
namespace mnemosy::graphics {
	enum PBRTextureType;
	enum NormapMapFormat;
	enum ChannelPackType;
	enum ChannelPackComponent;
	class Material;
	class Texture;

}

namespace mnemosy::systems
{

	class MaterialLibraryRegistry
	{
	public:
		MaterialLibraryRegistry();
		~MaterialLibraryRegistry();

		void LoadUserDirectoriesFromFile();
		void SaveUserDirectoriesData();

		FolderNode* AddNewFolder(FolderNode* parentNode,std::string& name);
		void RenameFolder(FolderNode* node, std::string& newName);
		void MoveFolder(FolderNode* dragSource, FolderNode* dragTarget);
		void DeleteAndKeepChildren(FolderNode* node);
		void DeleteFolderHierarchy(FolderNode* node);


		void AddNewMaterial(FolderNode* node, std::string& name);
		void RenameMaterial(FolderNode* node, systems::MaterialInfo* materialInfo, std::string& newName, int positionInVector);
		void DeleteMaterial(FolderNode* node, systems::MaterialInfo* materialInfo, int positionInVector);
		void MoveMaterial(FolderNode* sourceNode, FolderNode* targetNode, systems::MaterialInfo* materialInfo);


		void GenereateOpacityFromAlbedoAlpha(graphics::Material& activeMat);

		void GenerateChannelPackedTexture(graphics::Material& activeMat,std::string& suffix, graphics::ChannelPackType packType, graphics::ChannelPackComponent packComponent_R, graphics::ChannelPackComponent packComponent_G,graphics::ChannelPackComponent packComponent_B, graphics::ChannelPackComponent packComponent_A, unsigned int width, unsigned int height);

		void DeleteChannelPackedTexture(graphics::Material& activeMat, std::string suffix);


		void LoadActiveMaterialFromFile(std::filesystem::path& materialDirectory, systems::MaterialInfo* materialInfo,FolderNode* parentNode);
		void LoadActiveMaterialFromFile_Multithreaded(std::filesystem::path& materialDirectory, systems::MaterialInfo* materialInfo,FolderNode* parentNode);

		void SaveActiveMaterialToFile();
		void SetDefaultMaterial();

		bool UserMaterialBound() { return m_userMaterialBound; }

		void LoadTextureForActiveMaterial(graphics::PBRTextureType textureType, std::string& filepath);
		void DeleteTextureOfActiveMaterial(graphics::PBRTextureType textureType);
		

		void OpenFolderNode(FolderNode* node);
		void ClearUserMaterialsAndFolders();
		void ClearInternalTree_OnlyMemory();

		bool LoadExistingMnemosyLibrary(std::filesystem::path& pathToDataFile, bool savePermanently, bool deleteCurrentLibrary);


		// Getters
		FolderNode* GetRootFolder();
		FolderNode* GetFolderByID(FolderNode* node, const unsigned int id);

		int GetActiveMaterialID() { return m_activeMaterialID; }
		FolderNode* GetSelectedNode() { return m_selectedFolderNode; }
		std::filesystem::path& GetActiveMaterialDataFilePath() { return m_activeMaterialDataFilePath; }
		std::filesystem::path GetActiveMaterialFolderPath();


		std::filesystem::path GetLibraryPath();
		std::filesystem::path GetFolderPath(FolderNode* node);
		std::filesystem::path GetMaterialPath(FolderNode* folderNode, MaterialInfo* matInfo);

		std::vector<std::string> GetFilepathsOfActiveMat(graphics::Material& activeMat);


		bool SearchMaterialsForKeyword(const std::string& keyword);
		std::vector<systems::MaterialInfo*>& GetSearchResultsList();

	private:
		bool CheckDataFile(const std::filesystem::path& dataFilePath);
		void CreateNewMaterialDataFile(std::filesystem::path& folderPath,std::string& name);
		void CreateDirectoryForNode(FolderNode* node);
		
	public:
		FolderTree* m_folderTree = nullptr;
		FolderNode* m_folderNodeOfActiveMaterial = nullptr;
	private:
		core::FileDirectories& m_fileDirectories;
		FolderNode* m_selectedFolderNode = nullptr;

		//std::filesystem::directory_entry m_userDirectoriesDataFile;
		
		// active Material;
		std::filesystem::path m_activeMaterialDataFilePath;
		unsigned int m_activeMaterialID = 0; // 0 means non selected // at startup and if selected gets deleted
		bool m_userMaterialBound = false;
		
		// data file 
		bool prettyPrintDataFile = false;
		bool prettyPrintMaterialFiles = false;
	};


} // !mnemosy::systems

#endif // !MATERIAL_LIBRARY_REGISTRY_H
