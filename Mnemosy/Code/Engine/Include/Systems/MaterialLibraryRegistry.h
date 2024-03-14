#ifndef MATERIAL_LIBRARY_REGISTRY_H
#define MATERIAL_LIBRARY_REGISTRY_H

#include <vector>
#include <string>
#include <filesystem>

#include <nlohmann/json.hpp>

using namespace nlohmann;
namespace fs = std::filesystem;

namespace mnemosy::systems {
	struct FolderNode;
}
namespace mnemosy::core {
	class FileDirectories;
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
		float emission_b		= 0.0f;
		float emission_g		= 0.0f;
		float emissionStrength	= 0.0f;
		
		float normalStrength	= 1.0f;
		float uvScale_x			= 1.0f;
		float uvScale_y			= 1.0f;

		bool albedoAssigned = false;
		std::string albedoPath	= "notAssigned";
		bool roughAssigned = false;
		std::string roughPath	= "notAssigned";
		bool metalAssigned = false;
		std::string metalPath	= "notAssigned";
		bool emissionAssigned = false;
		std::string emissionPath = "notAssigned";
		bool normalAssigned = false;
		std::string normalPath	= "notAssigned";
		bool aoAssigned = false;
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
		void ChangeMaterialName(systems::FolderNode* node, std::string& materialName, std::string& newName, int positionInVector);
		void DeleteMaterial(FolderNode* node, std::string& materialName, int positionInVector);
		void MoveMaterial(FolderNode* sourceNode, FolderNode* targetNode, std::string& name);
	private:
		core::FileDirectories& m_fileDirectories;
		// runtime id's are only used for identification at runtime. specifically for drag and drop behavior
		unsigned int m_runtimeIDCounter = 0;
		FolderNode* m_rootFolderNode;


		void CreateNewMaterialDataFile(fs::path& folderPath,std::string& name);
		void CreateDirectoryForNode(FolderNode* node);
		void RecursivUpadtePathFromRoot(FolderNode* node);
		void RecursivCleanFolderTreeMemory(FolderNode* node);


		// data file 
		bool prettyPrintDataFile = true;
		fs::directory_entry m_userDirectoriesDataFile;
		std::string m_rootNodeName = "Root";
		void RecursivLoadDirectories(FolderNode* node, json& json);
		json RecursivSaveDirectories(FolderNode* node);
		bool CheckDataFile(fs::directory_entry dataFile);
		
	};


} // !mnemosy::systems

#endif // !MATERIAL_LIBRARY_REGISTRY_H
