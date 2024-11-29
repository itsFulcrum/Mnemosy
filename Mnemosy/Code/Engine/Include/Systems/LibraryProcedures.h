#ifndef LIBRARY_PROCEDURES_H
#define LIBRARY_PROCEDURES_H

#include <string>
#include <filesystem>


namespace mnemosy::systems {
	enum LibEntryType;
	struct LibEntry;
	struct FolderNode;
}

namespace mnemosy::graphics {
	class UnlitMaterial;
	class Skybox;
	class PbrMaterial;
}

namespace mnemosy::systems {


	class LibProcedures {
	public:


		static void CreateDirectoryForFolderNode(systems::FolderNode* node);
		static bool CheckDataFile(const std::filesystem::path& dataFilePath);


		static std::filesystem::path LibEntry_GetFolderPath(systems::LibEntry* libEntry);
		static std::filesystem::path LibEntry_GetDataFilePath(systems::LibEntry* libEntry);


		static void LibEntry_PbrMaterial_CreateNewDataFile(systems::LibEntry* libEntry, bool prettyPrint);
		static void LibEntry_UnlitMaterial_CreateNewDataFile(systems::LibEntry* libEntry,bool prettyPring);
		static void LibEntry_Skybox_CreateNewDataFile( systems::LibEntry* libEntry,bool prettyPring);

		
		static void LibEntry_PbrMaterial_SaveToFile(systems::LibEntry* libEntry, graphics::PbrMaterial* pbrMat,bool prettyPrint);
		static void LibEntry_UnlitMaterial_SaveToFile(systems::LibEntry* libEntry, graphics::UnlitMaterial* unlitMat,bool prettyPrint);
		//TODO: implement
		static void LibEntry_SkyboxMaterial_SaveToFile(systems::LibEntry* libEntry, graphics::Skybox* skybox,bool prettyPrint);
		

		// rename data file and all textures stored within the material
		static void LibEntry_PbrMaterial_RenameFiles(LibEntry* libEntry, std::filesystem::path& entryFolderOldNamePath,  std::string& oldName, bool prettyPrint);
		static void LibEntry_UnlitMaterial_RenameFiles(LibEntry* libEntry, std::filesystem::path& entryFolderOldNamePath, std::string& oldName, bool prettyPrint);
		// TODO: implement
		static void LibEntry_SkyboxMaterial_RenameFiles(LibEntry* libEntry, std::filesystem::path& entryFolderOldNamePath, std::string& oldName, bool prettyPrint);



		static graphics::PbrMaterial* LoadPbrMaterialFromFile_Multithreaded(systems::LibEntry* libEntry, bool prettyPrint);
		static graphics::UnlitMaterial* LibEntry_UnlitMaterial_LoadFromFile(systems::LibEntry* libEntry, bool prettyPrint);


	};



}

#endif // !LIBRARY_PROCEDURES_H
