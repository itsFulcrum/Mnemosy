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
		MNSY_NORMAL_FORMAT_OPENGL = 0,
		MNSY_NORMAL_FORMAT_DIRECTX = 1
	};	

	// convert to this in the future
	enum ImageFileFormat {
		MNSY_FILE_FORMAT_TIF  = 0, 
		MNSY_FILE_FORMAT_PNG  = 1, 
		MNSY_FILE_FORMAT_JPG  = 2, 
		MNSY_FILE_FORMAT_HDR  = 3, 
		MNSY_FILE_FORMAT_EXR  = 4, 
		MNSY_FILE_FORMAT_KTX2 = 5, 
		MNSY_FILE_FORMAT_COUNT,
		MNSY_FILE_FORMAT_NONE
	};

	//// TODO: Remove this and switch to the upper enum 
	//enum ExportImageFormat {
	//	MNSY_TIF = 0,
	//	MNSY_PNG = 1
	//};

	// the order here is important dont change it
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

// used for opening an windows explorer
#define readable_textureFormats_DialogFilter "All Files\0 * .*\0 TIF (*.tif)\0*.tif\0 PNG (*.png)\0*.png\0 JPG (*.jpg)\0*.jpg\0 TIFF (*.tiff)\0*.tiff\0 EXR (*.exr)\0*.exr\0 HDR (*.hdr)\0*.hdr\0,KTX2 (*.ktx2)\0*.ktx2\0"

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


	namespace TexDefinitions {
		const static char* ImageFileFormats_string[6] = { ".tif",".png",".jpg",".hdr",".exr",".ktx2" }; // must be ordered the same as enum 'ImageFileFormat'
		const static char* NormalMapFormats_string[2] = { "OpenGl", "DirectX" }; // must be ordered the same as enum 'NormalMapFormat'
		const static char* ChannelPackTypes_string[2] = { "R+G+B", "R+G+B+A" }; // must be ordered the same as enum 'ChannelPackType'

		// includes more bc some formats have more than one valid extention like jpeg/jpg
		// if we add one here we need to update the function 'get_imageFileFormat_from_fileExtentionString' below
		const static std::vector<std::string> ValidFileFormatsExtentions_string = { ".tif",".tiff",".png",".jpg",".jpeg",".exr",".hdr",".ktx2" }; 

		const static char* ChannelPackComponents_string[16] = { // must be ordered the same as enum 'ChannelPackComponent'
			"None",
			"Albedo R",
			"Albedo G",
			"Albedo B",
			"Normal R",
			"Normal G",
			"Normal B",
			"Emmissive R",
			"Emmissive G",
			"Emmissive B",
			"Roughness",
			"Smoothness",
			"Metallic",
			"Ambient Occlusion",
			"Height",
			"Opacity"
		};
	}


	// TODO: Move implementation to a cpp file
	class TexUtil {

	public:

		// PBRTextureType

		// Returns Empty string if PBRType is not found or not a type
		static std::string get_JsonMatKey_path_from_PBRTextureType(PBRTextureType textureType) {

			switch (textureType)
			{
			case PBRTextureType::MNSY_TEXTURE_ALBEDO: 			return jsonMatKey_albedoPath;	break;
			case PBRTextureType::MNSY_TEXTURE_ROUGHNESS:		return jsonMatKey_roughPath;	break;
			case PBRTextureType::MNSY_TEXTURE_METALLIC:			return jsonMatKey_metalPath;	break;
			case PBRTextureType::MNSY_TEXTURE_NORMAL:			return jsonMatKey_normalPath;	break;
			case PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION:	return jsonMatKey_aoPath;		break;
			case PBRTextureType::MNSY_TEXTURE_EMISSION:			return jsonMatKey_emissionPath;	break;
			case PBRTextureType::MNSY_TEXTURE_HEIGHT:			return jsonMatKey_heightPath;	break;
			case PBRTextureType::MNSY_TEXTURE_OPACITY:			return jsonMatKey_opacityPath;	break;
			default: return std::string() ;break;
			}

			return std::string();

		}
		
		// Returns Empty string if PBRType is not found or not a type
		static std::string get_JsonMatKey_assigned_from_PBRTextureType(PBRTextureType textureType) {

			switch (textureType)
			{
			case PBRTextureType::MNSY_TEXTURE_ALBEDO: 			return jsonMatKey_albedoAssigned;	break;
			case PBRTextureType::MNSY_TEXTURE_ROUGHNESS:		return jsonMatKey_roughAssigned;	break;
			case PBRTextureType::MNSY_TEXTURE_METALLIC:			return jsonMatKey_metalAssigned;	break;
			case PBRTextureType::MNSY_TEXTURE_NORMAL:			return jsonMatKey_normalAssigned;	break;
			case PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION:	return jsonMatKey_aoAssigned;		break;
			case PBRTextureType::MNSY_TEXTURE_EMISSION:			return jsonMatKey_emissionAssigned;	break;
			case PBRTextureType::MNSY_TEXTURE_HEIGHT:			return jsonMatKey_heightAssigned;	break;
			case PBRTextureType::MNSY_TEXTURE_OPACITY:			return jsonMatKey_opacityAssigned;	break;
			default: return std::string() ;break;
			}

			return std::string();
		}
		
		// Returns Empty string if PBRType is not found or not a type
		static std::string get_filename_from_PBRTextureType(std::string& materialName,PBRTextureType textureType) {
			
			switch (textureType)
			{
			case PBRTextureType::MNSY_TEXTURE_ALBEDO: 			return materialName + texture_fileSuffix_albedo;	break;
			case PBRTextureType::MNSY_TEXTURE_ROUGHNESS:		return materialName + texture_fileSuffix_roughness;	break;
			case PBRTextureType::MNSY_TEXTURE_METALLIC:			return materialName + texture_fileSuffix_metallic;	break;
			case PBRTextureType::MNSY_TEXTURE_NORMAL:			return materialName + texture_fileSuffix_normal;	break;
			case PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION:	return materialName + texture_fileSuffix_ambientOcclusion;	break;
			case PBRTextureType::MNSY_TEXTURE_EMISSION:			return materialName + texture_fileSuffix_emissive;	break;
			case PBRTextureType::MNSY_TEXTURE_HEIGHT:			return materialName + texture_fileSuffix_height;	break;
			case PBRTextureType::MNSY_TEXTURE_OPACITY:			return materialName + texture_fileSuffix_opacity;	break;
			default: return std::string() ;break;
			}

			return std::string();
		}

		// Returns Empty string if PBRType is not found or not a type
		static std::string get_string_from_PBRTextureType(PBRTextureType type) {

			switch (type)
			{
			case PBRTextureType::MNSY_TEXTURE_ALBEDO:			return "Albedo"; 			break;
			case PBRTextureType::MNSY_TEXTURE_ROUGHNESS:		return "Roughness";			break;
			case PBRTextureType::MNSY_TEXTURE_METALLIC:			return "Metallic"; 			break;
			case PBRTextureType::MNSY_TEXTURE_NORMAL:			return "Normal"; 			break;
			case PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION:	return "Ambient Occlusion";	break;
			case PBRTextureType::MNSY_TEXTURE_EMISSION:			return "Emissive"; 			break;
			case PBRTextureType::MNSY_TEXTURE_HEIGHT:			return "Height"; 			break;
			case PBRTextureType::MNSY_TEXTURE_OPACITY:			return "Opacity"; 			break;
			case PBRTextureType::MNSY_TEXTURE_COUNT:			return "Count";				break;
			default: return std::string() ;break;
			}

			return std::string();
		}
		
		// Trys to find substrings like 'albedo' in a filename and returns the apropriate type like MNSY_ALBEDO, returns PBRTextureType::MNSY_NONE if it cant match the filename
		static PBRTextureType get_PBRTextureType_from_filename(const std::string& name) {

			std::string testAgainst = name;	
			std::transform(testAgainst.begin(), testAgainst.end(), testAgainst.begin(), [](unsigned char c) { return std::tolower(c); });

			//albdeo
			const char* albedos[4] = { "albedo","_col","color","diffuse"};


			for (int i = 0; i < 4; i++) {

				if (testAgainst.find(albedos[i]) != std::string::npos) {

					return PBRTextureType::MNSY_TEXTURE_ALBEDO;
				}
			}


			// Roughness
			const char* rough[3] = { "roughness","gloss","smoothness" };
			for (int i = 0; i < 3; i++) {

				if (testAgainst.find(rough[i]) != std::string::npos) {
					
					return PBRTextureType::MNSY_TEXTURE_ROUGHNESS;
				}
			}

			// metal
			if (testAgainst.find("metallic") != std::string::npos) {

				return PBRTextureType::MNSY_TEXTURE_METALLIC;
			}

			// normal
			const char* norm[2] = { "_normal" , "_nrm"};
			for (int i = 0; i < 2; i++) {

				if (testAgainst.find(norm[i]) != std::string::npos) {

					return PBRTextureType::MNSY_TEXTURE_NORMAL;
				}
			}

			// AO
			const char* ao[4] = { "_ao" , "_occ","_ambient","ambientocclusion" };
			for (int i = 0; i < 4; i++) {

				if (testAgainst.find(ao[i]) != std::string::npos) {

					return PBRTextureType::MNSY_TEXTURE_AMBIENTOCCLUSION;
				}
			}

			// emission
			const char* emit[2] = { "emission" , "emissive" };
			for (int i = 0; i < 2; i++) {

				if (testAgainst.find(emit[i]) != std::string::npos) {

					return PBRTextureType::MNSY_TEXTURE_EMISSION;
				}
			}

			// height
			const char* height[5] = { "height" , "depth", "bump", "_disp", "displacement"};
			for (int i = 0; i < 5; i++) {

				if (testAgainst.find(height[i]) != std::string::npos) {

					return PBRTextureType::MNSY_TEXTURE_HEIGHT;
				}
			}

			// opacity
			const char* opacity[3] = { "opacity" , "transparency", "_alpha" };
			for (int i = 0; i < 3; i++) {

				if (testAgainst.find(opacity[i]) != std::string::npos) {

					return PBRTextureType::MNSY_TEXTURE_OPACITY;
				}
			}


			return PBRTextureType::MNSY_TEXTURE_NONE;
		}

		// channel Pack Type
		static std::string get_string_from_channelPackComponent(ChannelPackComponent packComponent) {

			switch (packComponent)
			{
			case (MNSY_PACKCOMPONENT_NONE):		return std::string("None");		break;
			case (MNSY_PACKCOMPONENT_ALBEDO_R):	return std::string("Albedo_R");	break;
			case (MNSY_PACKCOMPONENT_ALBEDO_G):	return std::string("Albedo_G");	break;
			case (MNSY_PACKCOMPONENT_ALBEDO_B):	return std::string("Albedo_B");	break;

			case (MNSY_PACKCOMPONENT_NORMAL_R):	return std::string("Normal_R");	break;
			case (MNSY_PACKCOMPONENT_NORMAL_G):	return std::string("Normal_G");	break;
			case (MNSY_PACKCOMPONENT_NORMAL_B):	return std::string("Normal_B");	break;

			case (MNSY_PACKCOMPONENT_EMISSIVE_R): return std::string("Emissive_R");	break;
			case (MNSY_PACKCOMPONENT_EMISSIVE_G): return std::string("Emissive_G");	break;
			case (MNSY_PACKCOMPONENT_EMISSIVE_B): return std::string("Emissive_B");	break;

			case (MNSY_PACKCOMPONENT_ROUGHNESS):	return std::string("Roughness");  break;
			case (MNSY_PACKCOMPONENT_SMOOTHNESS):	return std::string("Smoothness"); break;
			case (MNSY_PACKCOMPONENT_METALLIC):		return std::string("Metallic");   break;
			case (MNSY_PACKCOMPONENT_AO):			return std::string("Ambient Occlusion");break;
			case (MNSY_PACKCOMPONENT_HEIGHT):		return std::string("Height");	break;
			case (MNSY_PACKCOMPONENT_OPACITY):		return std::string("Opacity");	break;

			default: return std::string("None");; break;
			}

			return std::string("NONE");
		}

		// Normal Map Format
		static std::string get_string_from_normalMapFormat(graphics::NormalMapFormat normalMapFormat) {
			return std::string(graphics::TexDefinitions::NormalMapFormats_string[(int)normalMapFormat]);
		}

		// Texture Format
		static std::string get_string_from_textureFormat(graphics::TextureFormat format) {

			switch (format)
			{
			case (MNSY_NONE): 	return std::string("None"); 	break;
			case (MNSY_R8):		return std::string("R8");		break;
			case (MNSY_RG8):	return std::string("RG8");		break;
			case (MNSY_RGB8):	return std::string("RGB8");		break;
			case (MNSY_RGBA8):	return std::string("RGBA8");	break;

			case (MNSY_R16):	return std::string("R16");		break;
			case (MNSY_RG16):	return std::string("RG16");		break;
			case (MNSY_RGB16):	return std::string("RGB16");	break;
			case (MNSY_RGBA16):	return std::string("RGBA16");	break;

			case (MNSY_R32):	return std::string("R32");		break;
			case (MNSY_RG32):	return std::string("RG32");		break;
			case (MNSY_RGB32):	return std::string("RGB32");	break;
			case (MNSY_RGBA32):	return std::string("RGBA32");	break;

			default:	return std::string("None"); break;
			}

			return std::string("NONE");
		}


		// returns given any format returns the corresponding format for a single channel so e.g. RGBA16 will return R16 and RG8 will return R8
		static TextureFormat get_channel_textureFormat(graphics::TextureFormat format) {
			
			switch (format)
			{
			case (TextureFormat::MNSY_NONE): 	return TextureFormat::MNSY_NONE;break;

			case (TextureFormat::MNSY_R8):		return TextureFormat::MNSY_R8;	break;
			case (TextureFormat::MNSY_RG8):		return TextureFormat::MNSY_R8;	break;
			case (TextureFormat::MNSY_RGB8):	return TextureFormat::MNSY_R8;	break;
			case (TextureFormat::MNSY_RGBA8):	return TextureFormat::MNSY_R8;	break;

			case (TextureFormat::MNSY_R16):		return TextureFormat::MNSY_R16;	break;
			case (TextureFormat::MNSY_RG16):	return TextureFormat::MNSY_R16;	break;
			case (TextureFormat::MNSY_RGB16):	return TextureFormat::MNSY_R16;	break;
			case (TextureFormat::MNSY_RGBA16):	return TextureFormat::MNSY_R16;	break;

			case (TextureFormat::MNSY_R32):		return TextureFormat::MNSY_R32;	break;
			case (TextureFormat::MNSY_RG32):	return TextureFormat::MNSY_R32;	break;
			case (TextureFormat::MNSY_RGB32):	return TextureFormat::MNSY_R32;	break;
			case (TextureFormat::MNSY_RGBA32):	return TextureFormat::MNSY_R32;	break;

			default:return TextureFormat::MNSY_NONE; break;
			}

			return TextureFormat::MNSY_NONE;
		}


		// returns the amount of channels from a texture format, returns 0 if format is MNSY_NONE
		static uint8_t get_channels_amount_from_textureFormat(const graphics::TextureFormat format){

			if (format == graphics::TextureFormat::MNSY_NONE) {
				return 0;
			}

			// amount of color channels
			uint8_t channels = (uint8_t)format % 4;
			if (channels == 0) {
				channels = 4;
			}
			return channels;
		}

		static void get_information_from_textureFormat(const graphics::TextureFormat format, uint8_t& outChannelsAmount, uint8_t& outBitsPerChannel, uint8_t& outBytesPerPixel) {

			outChannelsAmount = 0;
			outBitsPerChannel = 0;
			outBytesPerPixel = 0;

			if (format == graphics::TextureFormat::MNSY_NONE) {
				return;
			}

			// amount of color channels
			uint8_t channels = (uint8_t)format % 4;
			if (channels == 0) {
				channels = 4;
			}
			outChannelsAmount = channels;

			// asume 8 bits per pixel
			outBitsPerChannel = 8;
			outBytesPerPixel = channels * sizeof(uint8_t);

			if (format == graphics::MNSY_R16 || format == graphics::MNSY_RG16 || format == graphics::MNSY_RGB16 || format == graphics::MNSY_RGBA16) {
				outBitsPerChannel = 16;
				outBytesPerPixel = channels * sizeof(uint16_t);
			}
			else if (format == MNSY_R32 || format == MNSY_RG32 || format == MNSY_RGB32 || format == MNSY_RGBA32) {
				outBitsPerChannel = 32;
				outBytesPerPixel = channels * sizeof(uint32_t);
			}
		}
		
		// returns string representation of a file format like ImageFileFormat::MNSY_FILE_FORMAT_PNG -> ".png"
		static std::string get_string_from_imageFileFormat(const graphics::ImageFileFormat fileFormat) {
			return std::string(graphics::TexDefinitions::ImageFileFormats_string[(int)fileFormat]);
		}


		// returns ImageFileFormat::MNSY_FILE_FORMAT_NONE if none is found.
		static graphics::ImageFileFormat get_imageFileFormat_from_fileExtentionString(const std::string fileExtention) {

			// { ".tif", ".tiff", ".png", ".jpg", ".jpeg", ".exr", ".hdr", ".ktx2" }

			std::string ext = fileExtention;

			if 		(ext == ".tif" || ext == ".tiff") { return ImageFileFormat::MNSY_FILE_FORMAT_TIF;}
			else if (ext == ".png") { return ImageFileFormat::MNSY_FILE_FORMAT_PNG; }
			else if (ext == ".jpg" || ext == ".jpeg") { return ImageFileFormat::MNSY_FILE_FORMAT_JPG; }
			else if (ext == ".exr") { return ImageFileFormat::MNSY_FILE_FORMAT_EXR; }
			else if (ext == ".hdr") { return ImageFileFormat::MNSY_FILE_FORMAT_HDR; }
			else if (ext == ".ktx2") { return ImageFileFormat::MNSY_FILE_FORMAT_KTX2; }

			return ImageFileFormat::MNSY_FILE_FORMAT_NONE;
		}

		static bool is_image_file_extention_supported(std::string& extention) {

			for (size_t i = 0; i < graphics::TexDefinitions::ValidFileFormatsExtentions_string.size(); i++) {

				if (extention == graphics::TexDefinitions::ValidFileFormatsExtentions_string[i]) {
					return true;
				}
			}

			return false;
		}
	};


} // namepsace mnemosy::graphics

#endif // !TEXTURE_DEFINITIONS_H
