#ifndef MATERIAL_LIBRARY_REGISTRY_H
#define MATERIAL_LIBRARY_REGISTRY_H

#include <vector>
#include <string>
#include <filesystem>

#include <nlohmann/json.hpp>
using namespace nlohmann;


namespace fs = std::filesystem;

namespace mnemosy::systems
{

	struct MaterialEntry {
		std::string name;
		unsigned int id;
		std::string parentGroup; // ?? do we need ??

		float albedo_r			= 1.0f;
		float albedo_g			= 1.0f;
		float albedo_b			= 1.0f;
		
		float rough				= 0.5f;
		float metal				= 0.0f;
		
		float emission_r		= 0.0f;
		float emission_b		= 0.0f;
		float emission_g		= 0.0f;
		float emissionStrength	= 0.0f;
		
		float normalStrength	= 1.0f;
		float uvScale_x			= 1.0f;
		float uvScale_y			= 1.0f;

		std::string albedoPath	= "notAssigned";
		std::string roughPath	= "notAssigned";
		std::string metalPath	= "notAssigned";;
		std::string emissionPath = "notAssigned";
		std::string normalPath	= "notAssigned";
		std::string aoPath		= "notAssigned";

		std::string thumbnailPath = "notAssigned";
	};



	struct FolderNode {
		std::string name;
		FolderNode* parent;
		std::vector<FolderNode*> subNodes;
		std::string pathFromRoot;
		// maybe store full path .. could be useful
		// maybe store material entries associated with this folder

		
		FolderNode* GetSubNodeByName(std::string name);
		bool IsLeafNode();

		bool SubnodeExistsAlready(FolderNode* node, std::string name);

	};

	class MaterialLibraryRegistry
	{
	public:
		MaterialLibraryRegistry();
		~MaterialLibraryRegistry();

		FolderNode* GetRootFolder();

		FolderNode* CreateFolderNode(FolderNode* parentNode,std::string name);


		void DeleteFolderHierarchy(FolderNode* node);


		void SaveUserDirectoriesData();
		void LoadUserDirectoriesFromFile();

		bool CheckDataFile(fs::directory_entry dataFile);
	private:
		bool prettyPrintDataFile = true;


		void RecursivLoadDirectories(FolderNode* node, json& json);
		json RecursivSaveDirectories(FolderNode* node);
		void RecursivCleanFolderTreeMemory(FolderNode* node);


		FolderNode* m_rootFolderNode;

		fs::directory_entry m_userDirectoriesDataFile;
	};


} // !mnemosy::systems

#endif // !MATERIAL_LIBRARY_REGISTRY_H
