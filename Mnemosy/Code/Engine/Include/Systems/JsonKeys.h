#ifndef JSON_KEYS_H
#define JSON_KEYS_H


#include "string"

namespace mnemosy::systems {


// json identifiers for the UserLibraryDirectory file - containing a hierarchical tree structure 
#define jsonLibKey_RootNodeName		"Root"					// std::string - name of the root folder

#define jsonLibKey_MnemosyDataFile	"1_Mnemosy_Data_File"	// Top Level Json Object
#define jsonLibKey_HeaderInfo		"2_Header_Info"			// Json Object - Holding header Information
#define jsonLibKey_Description		"Description"			// std::string - just a string describing the purpose of the file
#define jsonLibKey_FolderTree		"3_UserDirectories"		// Json Obect - Contains the folder tree information in a treelike structure

#define jsonLibKey_name				"1_name"				// std::string - name of the folder
#define jsonLibKey_isLeaf			"2_isLeaf"				// bool - if the folder contains sub folders
#define jsonLibKey_pathFromRoot		"3_pathFromRoot"		// std::string - path to this folder from root folder
#define jsonLibKey_subFolderNames	"4_subFolderNames"		// std::vector<std::string> - list of names of the sub folders contained
#define jsonLibKey_subFolders		"5_subFolders"			// Json Object - Sub Tree of folders
#define jsonLibKey_hasMaterials		"6_hasMaterials"		// bool - does the folder contain materials
#define jsonLibKey_materialEntries	"6_materialEntries"		// std::vector<std::string> - list of names of the materials contained


// json identifiers for material data file
#define jsonMatKey_MnemosyDataFile	"1_Mnemosy_Data_File"	// std::string - header
#define jsonMatKey_MnemosyDataFileTxt "Material Meta Data"	// text description of header
#define jsonMatKey_name				"name"					// std:.string - name of the material

#define jsonMatKey_albedo_r			"albedo_r"				// float - value 0 to 1
#define jsonMatKey_albedo_g			"albedo_g"				// float - value 0 to 1
#define jsonMatKey_albedo_b			"albedo_b"				// float - value 0 to 1

#define jsonMatKey_roughness		"roughness"				// float - value 0 to 1
#define jsonMatKey_metallic			"metallic"				// float - value 0 to 1

#define jsonMatKey_emission_r		"emission_r"			// float - value 0 to 1
#define jsonMatKey_emission_g		"emission_g"			// float - value 0 to 1
#define jsonMatKey_emission_b		"emission_b"			// float - value 0 to 1
#define jsonMatKey_emissionStrength	"emissionStrength"		// float - value 0 to 1
#define jsonMatKey_useEmissiveAsMask "useEmissiveAsMask"	// bool  - whether to use emissive as a mask for the color value or not


#define jsonMatKey_normalStrength	"normalStrength"		// float - value
#define jsonMatKey_normalMapFormat	"normalMapFormat"		// int - format of the original provided normal map, 0 = OpenGl, 1 = DirectX
#define jsonMatKey_heightDepth		"heightDepth"			// float - value 
#define jsonMatKey_isSmoothness		"isSmoothness"			// bool - if texture was provided as smoothness texture

#define jsonMatKey_uvScale_x		"uvScale_x"				// float - value
#define jsonMatKey_uvScale_y		"uvScale_y"				// float - value
#define jsonMatKey_opacityThreshold "opacityThreshold"		// float - value 0 to 1


#define jsonMatKey_albedoAssigned	"albedoAssigned"		// bool - if a texture has been assinged
#define jsonMatKey_roughAssigned	"roughAssigned"			// bool - if a texture has been assinged
#define jsonMatKey_metalAssigned	"metalAssigned"			// bool - if a texture has been assinged
#define jsonMatKey_emissionAssigned	"emissionAssigned"		// bool - if a texture has been assinged
#define jsonMatKey_normalAssigned	"normalAssigned"		// bool - if a texture has been assinged
#define jsonMatKey_aoAssigned		"aoAssigned"			// bool - if a texture has been assinged
#define jsonMatKey_heightAssigned	"heightAssigned"		// bool - if a texture has been assinged
#define jsonMatKey_opacityAssigned	"opacityAssigned"		// bool - if a texture has been assinged



#define jsonMatKey_pathNotAssigned  "notAssigned"			// string to use when path has not been assigned yet

#define jsonMatKey_albedoPath		"albedoPath"			// std::string - filename of the texture in the same folder
#define jsonMatKey_roughPath		"roughPath"				// std::string - filename of the texture in the same folder
#define jsonMatKey_metalPath		"metalPath"				// std::string - filename of the texture in the same folder
#define jsonMatKey_emissionPath		"emissionPath"			// std::string - filename of the texture in the same folder
#define jsonMatKey_normalPath		"normalPath"			// std::string - filename of the texture in the same folder
#define jsonMatKey_aoPath			"aoPath"				// std::string - filename of the texture in the same folder
#define jsonMatKey_heightPath		"hightPath"				// std::string - filename of the texture in the same folder
#define jsonMatKey_opacityPath		"opacityPath"			// std::string - filename of the texture in the same folder

#define jsonMatKey_thumbnailPath	"thumbnailPath"			// std::string - filename of the texture in the same folder


} // namespace mnemosy::systems

#endif // !JSON_KEYS_H
