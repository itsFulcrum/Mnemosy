#ifndef TEXTURE_DEFINITIONS_H
#define TEXTURE_DEFINITIONS_H

#include "string"
#include "Include/Systems/JsonKeys.h"
#include "Include/Core/Log.h"


#include <cctype>
#include <clocale>

namespace mnemosy::graphics {

	enum PBRTextureType {
		MNSY_TEXTURE_ALBEDO				= 0,
		MNSY_TEXTURE_ROUGHNESS			= 1,
		MNSY_TEXTURE_METALLIC			= 2,
		MNSY_TEXTURE_NORMAL				= 3,
		MNSY_TEXTURE_AMBIENTOCCLUSION	= 4,
		MNSY_TEXTURE_EMISSION			= 5,
		MNSY_TEXTURE_HEIGHT				= 6,
		MNSY_TEXTURE_OPACITY			= 7,
		MNSY_TEXTURE_COUNT				= 8,
		MNSY_TEXTURE_NONE				= 9
	};
	enum NormalMapFormat {
		// FIXME: typo, should be OPENGL // fix in visual studio
		MNSY_NORMAL_FORMAT_OPENGl = 0,
		MNSY_NORMAL_FORMAT_DIRECTX = 1
	};

	enum ExportImageFormat {
		//MNSY_KTX2 = 0,
		MNSY_TIF = 0,
		MNSY_PNG = 1
	};


	enum TextureFormat {
		MNSY_NONE		= 0,
		MNSY_R8			= 1,
		MNSY_RG8		= 2,
		MNSY_RGB8		= 3,
		MNSY_RGBA8		= 4,

		MNSY_R16		= 5,
		MNSY_RG16		= 6,
		MNSY_RGB16		= 7,
		MNSY_RGBA16		= 8,

		MNSY_R32		= 9,
		MNSY_RG32		= 10,
		MNSY_RGB32		= 11,
		MNSY_RGBA32		= 12
	};

	enum ChannelPackType
	{
		MNSY_PACKTYPE_RGB = 0,
		MNSY_PACKTYPE_RGBA = 1
	};

	enum ChannelPackComponent
	{
		MNSY_PACKCOMPONENT_NONE = 0,

		MNSY_PACKCOMPONENT_ALBEDO_R		= 1,
		MNSY_PACKCOMPONENT_ALBEDO_G		= 2,
		MNSY_PACKCOMPONENT_ALBEDO_B		= 3,

		MNSY_PACKCOMPONENT_NORMAL_R		= 4,
		MNSY_PACKCOMPONENT_NORMAL_G		= 5,
		MNSY_PACKCOMPONENT_NORMAL_B		= 6,
		
		MNSY_PACKCOMPONENT_EMISSIVE_R	= 7,
		MNSY_PACKCOMPONENT_EMISSIVE_G	= 8,
		MNSY_PACKCOMPONENT_EMISSIVE_B	= 9,

		// After 10 should only be single channel types otherwise texture gen manager has to be adjusted
		MNSY_PACKCOMPONENT_ROUGHNESS	= 10,
		MNSY_PACKCOMPONENT_SMOOTHNESS	= 11,
		MNSY_PACKCOMPONENT_METALLIC		= 12,
		MNSY_PACKCOMPONENT_AO			= 13,
		MNSY_PACKCOMPONENT_HEIGHT		= 14,
		MNSY_PACKCOMPONENT_OPACITY		= 15

	};

#define readable_textureFormats_DialogFilter "All Files\0 * .*\0 TIF (*.tif)\0*.tif\0 PNG (*.png)\0*.png\0 JPG (*.jpg)\0*.jpg\0 TIFF (*.tiff)\0*.tiff\0"

#define texture_textureFileType ".tif"					// file type of textures stored on disk

#define fileSuffix_albedo			"_albedo"
#define fileSuffix_normal			"_normal"
#define fileSuffix_roughness		"_roughness"
#define fileSuffix_metallic			"_metallic"
#define fileSuffix_ambientOcclusion	"_ambientOcclusion"
#define fileSuffix_emissive			"_emissive"
#define fileSuffix_height			"_height"
#define fileSuffix_opacity			"_opacity"


#define texture_fileSuffix_albedo			"_albedo"			+ texture_textureFileType
#define texture_fileSuffix_normal			"_normal"			+ texture_textureFileType
#define texture_fileSuffix_roughness		"_roughness"		+ texture_textureFileType
#define texture_fileSuffix_metallic			"_metallic"			+ texture_textureFileType
#define texture_fileSuffix_ambientOcclusion	"_ambientOcclusion" + texture_textureFileType
#define texture_fileSuffix_emissive			"_emissive"			+ texture_textureFileType
#define texture_fileSuffix_height			"_height"			+ texture_textureFileType
#define texture_fileSuffix_opacity			"_opacity"			+ texture_textureFileType

#define texture_fileSuffix_thumbnail		"_thumbnail.ktx2"	// file name of the thumbnail texture (in ktx2 format)



	class TextureDefinitions {

	public:
		static std::string GetTextureFileNameFromTextureType(std::string& materialName,PBRTextureType textureType) {

			if (textureType == graphics::MNSY_TEXTURE_ALBEDO) {
				return materialName + texture_fileSuffix_albedo;
			}
			else if (textureType == graphics::MNSY_TEXTURE_NORMAL) {
				return materialName + texture_fileSuffix_normal;
			}
			else if (textureType == graphics::MNSY_TEXTURE_ROUGHNESS) {
				return materialName + texture_fileSuffix_roughness;
			}
			else if (textureType == graphics::MNSY_TEXTURE_METALLIC) {
				return materialName + texture_fileSuffix_metallic;
			}
			else if (textureType == graphics::MNSY_TEXTURE_AMBIENTOCCLUSION) {
				return materialName + texture_fileSuffix_ambientOcclusion;
			}
			else if (textureType == graphics::MNSY_TEXTURE_EMISSION) {
				return materialName + texture_fileSuffix_emissive;
			}
			else if (textureType == graphics::MNSY_TEXTURE_HEIGHT) {
				return materialName + texture_fileSuffix_height;
			}
			else if (textureType == graphics::MNSY_TEXTURE_OPACITY) {
				return materialName + texture_fileSuffix_opacity;
			}

			return std::string();
		}


		static std::string GetJsonMatKey_assigned_FromTextureType(PBRTextureType textureType) {

			if (textureType == graphics::MNSY_TEXTURE_ALBEDO) {
				return jsonMatKey_albedoAssigned;
			}
			else if (textureType == graphics::MNSY_TEXTURE_NORMAL) {
				return jsonMatKey_normalAssigned;
			}
			else if (textureType == graphics::MNSY_TEXTURE_ROUGHNESS) {
				return jsonMatKey_roughAssigned;
			}
			else if (textureType == graphics::MNSY_TEXTURE_METALLIC) {
				return jsonMatKey_metalAssigned;
			}
			else if (textureType == graphics::MNSY_TEXTURE_AMBIENTOCCLUSION) {
				return jsonMatKey_aoAssigned;
			}
			else if (textureType == graphics::MNSY_TEXTURE_EMISSION) {
				return jsonMatKey_emissionAssigned;
			}
			else if (textureType == graphics::MNSY_TEXTURE_HEIGHT) {
				return jsonMatKey_heightAssigned;
			}
			else if (textureType == graphics::MNSY_TEXTURE_OPACITY) {
				return jsonMatKey_opacityAssigned;
			}

			return std::string();
		}

		static std::string GetJsonMatKey_path_FromTextureType(PBRTextureType textureType) {

			if (textureType == graphics::MNSY_TEXTURE_ALBEDO) {
				return jsonMatKey_albedoPath;
			}
			else if (textureType == graphics::MNSY_TEXTURE_NORMAL) {
				return jsonMatKey_normalPath;
			}
			else if (textureType == graphics::MNSY_TEXTURE_ROUGHNESS) {
				return jsonMatKey_roughPath;
			}
			else if (textureType == graphics::MNSY_TEXTURE_METALLIC) {
				return jsonMatKey_metalPath;
			}
			else if (textureType == graphics::MNSY_TEXTURE_AMBIENTOCCLUSION) {
				return jsonMatKey_aoPath;
			}
			else if (textureType == graphics::MNSY_TEXTURE_EMISSION) {
				return jsonMatKey_emissionPath;
			}
			else if (textureType == graphics::MNSY_TEXTURE_HEIGHT) {
				return jsonMatKey_heightPath;
			}
			else if (textureType == graphics::MNSY_TEXTURE_OPACITY) {
				return jsonMatKey_opacityPath;
			}

			return std::string();

		}


		static std::string GetTextureNameFromEnumType(PBRTextureType type) {

			switch (type)
			{
			case mnemosy::graphics::MNSY_TEXTURE_ALBEDO:
				return "Albedo";
				break;
			case mnemosy::graphics::MNSY_TEXTURE_ROUGHNESS:
				return "Roughness";
				break;
			case mnemosy::graphics::MNSY_TEXTURE_METALLIC:
				return "Metallic";
				break;
			case mnemosy::graphics::MNSY_TEXTURE_NORMAL:
				return "Normal";
				break;
			case mnemosy::graphics::MNSY_TEXTURE_AMBIENTOCCLUSION:
				return "Ambient Occlusion";
				break;
			case mnemosy::graphics::MNSY_TEXTURE_EMISSION:
				return "Emissive";
				break;
			case mnemosy::graphics::MNSY_TEXTURE_HEIGHT:
				return "Height";
				break;
			case mnemosy::graphics::MNSY_TEXTURE_OPACITY:
				return "Opacity";
				break;
			case mnemosy::graphics::MNSY_TEXTURE_COUNT:
				return "Count";
				break;
			default:
				return "none";
				break;
			}

			return "none";
		}


		static bool IsImageFileExtentionValid(std::string& extention) {

			std::string validExtentions[5] = {".tif",".tiff",".png",".jpg",".jpeg"};


			for (size_t i = 0; i < 5; i++) {

				if (extention == validExtentions[i]) {

					return true;
				}

			}

			return false;

		}


		static std::string GetNameOfPackComponent(ChannelPackComponent packComponent) {

			switch (packComponent)
			{
			case (MNSY_PACKCOMPONENT_NONE):
				return std::string("None");
				break;
			case (MNSY_PACKCOMPONENT_ALBEDO_R):
				return std::string("Albedo_R");
				break;
			case (MNSY_PACKCOMPONENT_ALBEDO_G):
				return std::string("Albedo_G");
				break;
			case (MNSY_PACKCOMPONENT_ALBEDO_B):
				return std::string("Albedo_B");
				break;


			case (MNSY_PACKCOMPONENT_NORMAL_R):
				return std::string("Normal_R");
				break;
			case (MNSY_PACKCOMPONENT_NORMAL_G):
				return std::string("Normal_G");
				break;

			case (MNSY_PACKCOMPONENT_NORMAL_B):
				return std::string("Normal_B");
				break;


			case (MNSY_PACKCOMPONENT_EMISSIVE_R):
				return std::string("Emissive_R");
				break;
			case (MNSY_PACKCOMPONENT_EMISSIVE_G):
				return std::string("Emissive_G");
				break;
			case (MNSY_PACKCOMPONENT_EMISSIVE_B):
				return std::string("Emissive_B");
				break;
			case (MNSY_PACKCOMPONENT_ROUGHNESS):
				return std::string("Roughness");
				break;
			case (MNSY_PACKCOMPONENT_SMOOTHNESS):
				return std::string("Smoothness");
				break;
			case (MNSY_PACKCOMPONENT_METALLIC):
				return std::string("Metallic");
				break;
			case (MNSY_PACKCOMPONENT_AO):
				return std::string("Ambient Occlusion");
				break;
			case (MNSY_PACKCOMPONENT_HEIGHT):
				return std::string("Height");
				break;

			case (MNSY_PACKCOMPONENT_OPACITY):
				return std::string("Opacity");
				break;

			default:
				return std::string("None");;
				break;
			}

			return std::string("NONE");
		}


		static void str_tolower(std::string& s)
		{
			std::transform(s.begin(), s.end(), s.begin(),[](unsigned char c) { return std::tolower(c); } );
		}

		static PBRTextureType GetTypeFromFileName(const std::string& name) {

			std::string testAgainst = name;			
			str_tolower(testAgainst);			

			//albdeo
			const char* albedos[4] = { "albedo","_col","color","diffuse"};


			for (int i = 0; i < 4; i++) {

				if (testAgainst.find(albedos[i]) != std::string::npos) {

					return MNSY_TEXTURE_ALBEDO;
				}
			}


			// Roughness
			const char* rough[3] = { "roughness","gloss","smoothness" };
			for (int i = 0; i < 3; i++) {

				if (testAgainst.find(rough[i]) != std::string::npos) {
					
					return MNSY_TEXTURE_ROUGHNESS;
				}
			}

			// metal
			if (testAgainst.find("metallic") != std::string::npos) {

				return MNSY_TEXTURE_METALLIC;
			}

			// normal
			const char* norm[2] = { "_normal" , "_nrm"};
			for (int i = 0; i < 2; i++) {

				if (testAgainst.find(norm[i]) != std::string::npos) {

					return MNSY_TEXTURE_NORMAL;
				}
			}

			// AO
			const char* ao[4] = { "_ao" , "_occ","_ambient","ambientocclusion" };
			for (int i = 0; i < 4; i++) {

				if (testAgainst.find(ao[i]) != std::string::npos) {

					return MNSY_TEXTURE_AMBIENTOCCLUSION;
				}
			}

			// emission
			const char* emit[2] = { "emission" , "emissive" };
			for (int i = 0; i < 2; i++) {

				if (testAgainst.find(emit[i]) != std::string::npos) {

					return MNSY_TEXTURE_EMISSION;
				}
			}

			// height
			const char* height[5] = { "height" , "depth", "bump", "_disp", "displacement"};
			for (int i = 0; i < 5; i++) {

				if (testAgainst.find(height[i]) != std::string::npos) {

					return MNSY_TEXTURE_HEIGHT;
				}
			}

			// opacity
			const char* opacity[3] = { "opacity" , "transparency", "_alpha" };
			for (int i = 0; i < 3; i++) {

				if (testAgainst.find(opacity[i]) != std::string::npos) {

					return MNSY_TEXTURE_OPACITY;
				}
			}


			return PBRTextureType::MNSY_TEXTURE_NONE;
		}

	};


} // namepsace mnemosy::graphics

#endif // !TEXTURE_DEFINITIONS_H
