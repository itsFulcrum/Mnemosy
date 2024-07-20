#ifndef TEXTURE_DEFINITIONS_H
#define TEXTURE_DEFINITIONS_H

#include "string"
#include "Include/Systems/JsonKeys.h"

namespace mnemosy::graphics {

	enum PBRTextureType {
		MNSY_TEXTURE_ALBEDO				= 0,
		MNSY_TEXTURE_ROUGHNESS			= 1,
		MNSY_TEXTURE_METALLIC			= 2,
		MNSY_TEXTURE_NORMAL				= 3,
		MNSY_TEXTURE_AMBIENTOCCLUSION	= 4,
		MNSY_TEXTURE_EMISSION			= 5,
		// not implemented yet
		MNSY_TEXTURE_HEIGHT				= 6,
		MNSY_TEXTURE_OPACITY			= 7,
		MNSY_TEXTURE_CUSTOMPACKED		= 8
	};
	enum NormalMapFormat {
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


#define texture_textureFileType ".tif"					// file type of textures stored on disk

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


	};


} // namepsace mnemosy::graphics

#endif // !TEXTURE_DEFINITIONS_H
