#ifndef MATERIAL_LIBRARY_REGISTRY_H
#define MATERIAL_LIBRARY_REGISTRY_H

#include <vector>
#include <string>
#include <filesystem>

namespace mnemosy::systems {
	enum LibEntryType;
	struct LibEntry;
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
	class UnlitMaterial;
	class PbrMaterial;
	class Texture;
}

namespace mnemosy::systems
{
	// TODO: make seperate file with file directory procedures that can be handle outside of this class so that it gets less crowded
	class MaterialLibraryRegistry
	{
	public:
		MaterialLibraryRegistry() = default;
		~MaterialLibraryRegistry() = default;

		void Init();
		void Shutdown();

		void LoadUserDirectoriesFromFile();
		void SaveUserDirectoriesData();

		FolderNode* AddNewFolder(FolderNode* parentNode,std::string& name);
		void RenameFolder(FolderNode* node, std::string& newName);
		void MoveFolder(FolderNode* dragSource, FolderNode* dragTarget);
		void DeleteAndKeepChildren(FolderNode* node);
		void DeleteFolderHierarchy(FolderNode* node);

		// TODO: make sure thumbnails get directly created for the different entry types
		void LibEntry_CreateNew(FolderNode* node,  LibEntryType type, std::string& name);
		void LibEntry_Rename(systems::LibEntry* libEntry, std::string& newName);
		void LibEntry_Delete(systems::LibEntry* libEntry, int positionInVector);
		void LibEntry_Move(FolderNode* sourceNode, FolderNode* targetNode, systems::LibEntry* libEntry);

		std::filesystem::path LibEntry_GetDataFilePath(LibEntry* libEntry);
		std::filesystem::path LibEntry_GetFolderPath(LibEntry* libEntry);
		
		// TODO: handle skybox Entry type
		void LibEntry_Load(systems::LibEntry* libEntry);

		bool IsActiveEntry(uint16_t runtimeID);
		systems::LibEntry* ActiveLibEntry_Get() { return m_activeLibEntry; }
		void ActiveLibEntry_SaveToFile();

		std::vector<std::string> ActiveLibEntry_GetTexturePaths();


		// For Pbr Material Only
		void ActiveLibEntry_PbrMat_GenerateOpacityFromAlbedoAlpha(LibEntry* libEntry, graphics::PbrMaterial& activeMat);
		void ActiveLibEntry_PbrMat_GenerateChannelPackedTexture(LibEntry* libEntry, graphics::PbrMaterial& activeMat,std::string& suffix, graphics::ChannelPackType packType, graphics::ChannelPackComponent packComponent_R, graphics::ChannelPackComponent packComponent_G,graphics::ChannelPackComponent packComponent_B, graphics::ChannelPackComponent packComponent_A, unsigned int width, unsigned int height,uint8_t bitDepth);
		void ActiveLibEntry_PbrMat_DeleteChannelPackedTexture(LibEntry* libEntry, graphics::PbrMaterial& activeMat, std::string suffix);

		void ActiveLibEntry_PbrMat_LoadTexture(graphics::PBRTextureType textureType, std::filesystem::path& filepath);
		void ActiveLibEntry_PbrMat_DeleteTexture(graphics::PBRTextureType textureType);

		void ActiveLibEntry_UnlitMat_LoadTexture(std::filesystem::path& filepath);
		void ActiveLibEntry_UnlitMat_DeleteTexture();

		void ActiveLibEntry_Skybox_LoadTexture(std::filesystem::path& filepath);
		void ActiveLibEntry_Skybox_DeleteTexture();


		void SetDefaultMaterial();
		bool UserEntrySelected() { return m_activeLibEntry != nullptr; }		

		void OpenFolderNode(FolderNode* node);
		void ClearUserMaterialsAndFolders();
		void ClearInternalTree_OnlyMemory();

		bool LoadExistingMnemosyLibrary(std::filesystem::path& pathToDataFile, bool savePermanently, bool deleteCurrentLibrary);


		// Getters
		FolderNode* GetRootFolder();
		FolderNode* GetFolderByID(FolderNode* node, const unsigned int id);
		FolderNode* GetSelectedNode() { return m_selectedFolderNode; }
		

		std::filesystem::path GetLibraryPath();
		std::filesystem::path Folder_GetFullPath(FolderNode* node);

		bool SearchLibEntriesForKeyword(const std::string& keyword);
		std::vector<systems::LibEntry*>& GetSearchResultsList();

		systems::LibEntryType GetEntryTypeToRenderWith();


	private:
		FolderTree* m_folderTree = nullptr;
		core::FileDirectories* m_fileDirectories = nullptr;
		FolderNode* m_selectedFolderNode = nullptr;

		systems::LibEntry* m_activeLibEntry = nullptr;
		systems::LibEntryType m_lastActiveMaterialLibEntry;
		
		// data file 
		bool prettyPrintDataFile = false;
		bool prettyPrintMaterialFiles = false;
	public:

		bool inSearchMode = false; // accessed by library gui panel and contents gui panel
	};


} // !mnemosy::systems

#endif // !MATERIAL_LIBRARY_REGISTRY_H
