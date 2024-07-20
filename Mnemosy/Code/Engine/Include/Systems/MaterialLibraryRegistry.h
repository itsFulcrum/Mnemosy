#ifndef MATERIAL_LIBRARY_REGISTRY_H
#define MATERIAL_LIBRARY_REGISTRY_H

#include <vector>
#include <string>
#include <filesystem>

#include <nlohmann/json.hpp>

using namespace nlohmann;
namespace fs = std::filesystem;

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
	class Material;
}

namespace mnemosy::systems
{

	struct MaterialEntry {
		std::string name;

		float albedo_r			= 1.0f;
		float albedo_g			= 1.0f;
		float albedo_b			= 1.0f;
		
		float roughness			= 0.5f;
		float metallic			= 0.0f;
		
		float emission_r		= 0.0f;
		float emission_g		= 0.0f;
		float emission_b		= 0.0f;
		float emissionStrength	= 0.0f;
		
		float normalStrength	= 1.0f;
		float uvScale_x			= 1.0f;
		float uvScale_y			= 1.0f;

		bool albedoAssigned = false;
		bool roughAssigned = false;
		bool metalAssigned = false;
		bool emissionAssigned = false;
		bool normalAssigned = false;
		bool aoAssigned = false;

		std::string albedoPath	= "notAssigned";
		std::string roughPath	= "notAssigned";
		std::string metalPath	= "notAssigned";
		std::string emissionPath = "notAssigned";
		std::string normalPath	= "notAssigned";
		std::string aoPath		= "notAssigned";

		std::string thumbnailPath = "notAssigned";
	};

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
		void DeleteAndKeepChildren(FolderNode* node); // TODO
		void DeleteFolderHierarchy(FolderNode* node);

		FolderNode* GetRootFolder();
		FolderNode* GetFolderByID(FolderNode* node, const unsigned int id);


		void AddNewMaterial(FolderNode* node, std::string& name);
		void RenameMaterial(FolderNode* node, systems::MaterialInfo& materialInfo, std::string& newName, int positionInVector);
		void DeleteMaterial(FolderNode* node, systems::MaterialInfo& materialInfo, int positionInVector);
		void MoveMaterial(FolderNode* sourceNode, FolderNode* targetNode, systems::MaterialInfo& materialInfo);



		void LoadActiveMaterialFromFile(fs::path& materialDirectory, systems::MaterialInfo& materialInfo,FolderNode* parentNode);
		void SaveActiveMaterialToFile();
		void SetDefaultMaterial();

		FolderNode* GetSelectedNode() { return m_selectedFolderNode; }
		int GetActiveMaterialID() { return m_activeMaterialID; }
		bool UserMaterialBound() { return m_userMaterialBound; }
		fs::path& GetActiveMaterialDataFilePath() { return m_activeMaterialDataFilePath; }
		fs::path GetActiveMaterialFolderPath();


		void LoadTextureForActiveMaterial(graphics::PBRTextureType textureType, std::string& filepath);
		void DeleteTextureOfActiveMaterial(graphics::PBRTextureType textureType);
		
		fs::path GetLibraryPath();
		fs::path GetFolderPath(FolderNode* node);
		fs::path GetMaterialPath(FolderNode* folderNode, MaterialInfo& matInfo);


		void OpenFolderNode(FolderNode* node);
		void ClearUserMaterialsAndFolders();

		std::vector<std::string> GetFilepathsOfActiveMat(graphics::Material& activeMat);

	private:
		bool CheckDataFile(fs::directory_entry dataFile);
		void CreateNewMaterialDataFile(fs::path& folderPath,std::string& name);
		void CreateDirectoryForNode(FolderNode* node);

	public:
		FolderTree* m_folderTree = nullptr;
		FolderNode* m_folderNodeOfActiveMaterial = nullptr;
	private:
		core::FileDirectories& m_fileDirectories;
		FolderNode* m_selectedFolderNode = nullptr;

		// data file 
		bool prettyPrintDataFile = false;
		bool prettyPrintMaterialFiles = true;
		fs::directory_entry m_userDirectoriesDataFile;
		
		// active Material;
		fs::path m_activeMaterialDataFilePath;
		unsigned int m_activeMaterialID = 0; // 0 means non selected // at startup and if selected gets deleted
		bool m_userMaterialBound = false;
	};


} // !mnemosy::systems

#endif // !MATERIAL_LIBRARY_REGISTRY_H
