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
}
namespace mnemosy::core {
	class FileDirectories;
}
namespace mnemosy::graphics {
	enum PBRTextureType;
	enum NormapMapFormat;
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
		
		
		
		void RenameDirectory(FolderNode* node,std::string oldPathFromRoot);
		void MoveDirectory(FolderNode* dragSource, FolderNode* dragTarget);
		void DeleteFolderHierarchy(FolderNode* node);

		FolderNode* CreateFolderNode(FolderNode* parentNode,std::string name);

		FolderNode* GetRootFolder();
		FolderNode* RecursivGetNodeByRuntimeID(FolderNode* node, unsigned int id);

		void CreateNewMaterial(FolderNode* node, std::string name);
		void ChangeMaterialName(FolderNode* node, systems::MaterialInfo& materialInfo, std::string& newName, int positionInVector);
		void DeleteMaterial(FolderNode* node, systems::MaterialInfo& materialInfo, int positionInVector);
		void MoveMaterial(FolderNode* sourceNode, FolderNode* targetNode, systems::MaterialInfo& materialInfo);

		void LoadActiveMaterialFromFile(fs::path& materialDirectory, systems::MaterialInfo& materialInfo,FolderNode* parentNode);
		void SaveActiveMaterialToFile();
		void SetDefaultMaterial();

		FolderNode* GetSelectedNode() { return m_selectedFolderNode; }
		int GetActiveMaterialID() { return m_activeMaterialID; }
		bool UserMaterialBound() { return m_userMaterialBound; }
		fs::path& GetActiveMaterialDataFilePath() { return m_activeMaterialDataFilePath; }
		//std::string& GetFolderNodeNameOfActiveMaterial() { return m_folderNodeOfActiveMaterial->name; }
		void LoadTextureForActiveMaterial(graphics::PBRTextureType textureType, std::string& filepath);
		void DeleteTextureOfActiveMaterial(graphics::PBRTextureType textureType);


		void OpenFolderNode(FolderNode* node);
		void ClearUserMaterialsAndFolders();
	private:

		core::FileDirectories& m_fileDirectories;
		// runtime id's are only used for identification at runtime. specifically for drag and drop behavior
		unsigned int m_runtimeIDCounter = 1;
		unsigned int m_runtimeMaterialIDCounter = 1;
		FolderNode* m_rootFolderNode;
		FolderNode* m_selectedFolderNode = nullptr;

		void CreateNewMaterialDataFile(fs::path& folderPath,std::string& name);
		void CreateDirectoryForNode(FolderNode* node);
		void RecursivUpadtePathFromRoot(FolderNode* node);
		void RecursivCleanFolderTreeMemory(FolderNode* node);


		// data file 
		bool prettyPrintDataFile = false;
		bool prettyPrintMaterialFiles = true;
		fs::directory_entry m_userDirectoriesDataFile;
		std::string m_rootNodeName = "Root";
		void RecursivLoadDirectories(FolderNode* node, json& json);
		json RecursivSaveDirectories(FolderNode* node);
		bool CheckDataFile(fs::directory_entry dataFile);
		
		// active Material;
		fs::path m_activeMaterialDataFilePath;
		unsigned int m_activeMaterialID = 0; // 0 means non selected // at startup and if selected gets deleted
		bool m_userMaterialBound = false;
	public:

		FolderNode* m_folderNodeOfActiveMaterial = nullptr;







	};


} // !mnemosy::systems

#endif // !MATERIAL_LIBRARY_REGISTRY_H
