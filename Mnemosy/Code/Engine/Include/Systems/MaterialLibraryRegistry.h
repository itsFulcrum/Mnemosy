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

	struct LibCollection {
		std::filesystem::path folderPath;
		std::string name;
	};

	class MaterialLibraryRegistry
	{
	public:
		MaterialLibraryRegistry() = default;
		~MaterialLibraryRegistry() = default;

		void Init();
		void Shutdown();

		void SaveCurrentSate();

		FolderNode* AddNewFolder(FolderNode* parentNode,std::string& name);
		void RenameFolder(FolderNode* node, std::string& newName);
		void MoveFolder(FolderNode* dragSource, FolderNode* dragTarget);
		void DeleteAndKeepChildren(FolderNode* node);
		void DeleteFolderHierarchy(FolderNode* node);

		void LibEntry_CreateNew(FolderNode* node,  LibEntryType type, std::string& name);
		void LibEntry_Rename(systems::LibEntry* libEntry, std::string& newName);
		void LibEntry_Delete(systems::LibEntry* libEntry, unsigned int positionInVector);
		void LibEntry_Move(FolderNode* sourceNode, FolderNode* targetNode, systems::LibEntry* libEntry);

		std::filesystem::path LibEntry_GetDataFilePath(LibEntry* libEntry);
		std::filesystem::path LibEntry_GetFolderPath(LibEntry* libEntry);
		
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
		bool UserEntrySelected();

		void OpenFolderNode(FolderNode* node);


		// Getters
		FolderNode* GetRootFolder();
		FolderNode* GetFolderByID(FolderNode* node, const unsigned int id);
		FolderNode* GetSelectedNode() { return m_selectedFolderNode; }
		
		std::filesystem::path Folder_GetFullPath(FolderNode* node);

		bool SearchLibEntriesForKeyword(const std::string& keyword);
		std::vector<systems::LibEntry*>& GetSearchResultsList();

		systems::LibEntryType GetEntryTypeToRenderWith();


		// Lib Collections
		const bool LibCollections_IsAnyActive() { return m_folderTree != nullptr; }


		void LibCollections_CreateNewEntry(const std::string& name, const std::filesystem::path& folderPath);
		void LibCollections_CreateNewEntryFromExisting(const std::string& name, const std::filesystem::path& dataFilePath);
		void LibCollections_RenameEntry(const unsigned int index, const std::string& newName);
		void LibCollections_RemoveEntryFromList(const unsigned int index);

		const std::vector<LibCollection>&  LibCollections_GetListVector() const { return  m_libCollectionsList; }
		const unsigned int LibCollections_GetCurrentSelectedID() const { return m_libCollection_currentSlected_id; }
		std::string LibCollections_MakeNameUnique(const std::string& name);
		void LibCollections_SwitchActiveCollection(const unsigned int index);
		
		std::filesystem::path ActiveLibCollection_GetFolderPath();
		std::filesystem::path ActiveLibCollection_GetDataFilePath();
		const std::string ActiveLibCollection_GetName();

	private:

		const bool LibCollection_LoadIntoActiveTree(std::filesystem::path& folderPath);
		void ActiveLibCollection_SaveToFile();
		void ActiveLibCollection_Unload();
		
		void LibCollections_SaveToFile();
		void LibCollections_LoadFromFile();

	private:

		// Lib Collection
		std::vector<LibCollection> m_libCollectionsList;
		int m_libCollection_currentSlected_id = -1; // if -1 it means none is selected

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
