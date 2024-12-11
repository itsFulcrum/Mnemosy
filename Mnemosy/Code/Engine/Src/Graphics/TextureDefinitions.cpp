#include "Include/Graphics/TextureDefinitions.h"

#include <glad/glad.h>


namespace mnemosy::graphics {

	// these have to be here bc of glad openGl include or some shit.

	uint32_t TexUtil::get_glInternalFormat_from_textureFormat(const graphics::TextureFormat format)
	{
		switch (format)
		{
		case mnemosy::graphics::MNSY_NONE:		return 0;

		case mnemosy::graphics::MNSY_R8:		return GL_R8;
		case mnemosy::graphics::MNSY_RG8:		return GL_RG8;
		case mnemosy::graphics::MNSY_RGB8:		return GL_RGB8;
		case mnemosy::graphics::MNSY_RGBA8:		return GL_RGBA8;

		case mnemosy::graphics::MNSY_R16:		return GL_R16;
		case mnemosy::graphics::MNSY_RG16:		return GL_RG16;
		case mnemosy::graphics::MNSY_RGB16:		return GL_RGB16;
		case mnemosy::graphics::MNSY_RGBA16:	return GL_RGBA16;

		case mnemosy::graphics::MNSY_R32:		return GL_R32F;
		case mnemosy::graphics::MNSY_RG32:		return GL_RG32F;
		case mnemosy::graphics::MNSY_RGB32:		return GL_RGB32F;
		case mnemosy::graphics::MNSY_RGBA32:	return GL_RGBA32F;

		default:
			return 0;
		}

		return 0;
	}

	uint32_t TexUtil::get_glFormat_from_textureFormat(const graphics::TextureFormat format)
	{
		switch (format)
		{
		case mnemosy::graphics::MNSY_NONE:		return 0;

		case mnemosy::graphics::MNSY_R8:		return GL_RED;
		case mnemosy::graphics::MNSY_RG8:		return GL_RG;
		case mnemosy::graphics::MNSY_RGB8:		return GL_RGB;
		case mnemosy::graphics::MNSY_RGBA8:		return GL_RGBA;

		case mnemosy::graphics::MNSY_R16:		return GL_RED;
		case mnemosy::graphics::MNSY_RG16:		return GL_RG;
		case mnemosy::graphics::MNSY_RGB16:		return GL_RGB;
		case mnemosy::graphics::MNSY_RGBA16:	return GL_RGBA;

		case mnemosy::graphics::MNSY_R32:		return GL_RED;
		case mnemosy::graphics::MNSY_RG32:		return GL_RG;
		case mnemosy::graphics::MNSY_RGB32:		return GL_RGB;
		case mnemosy::graphics::MNSY_RGBA32:	return GL_RGBA;

		default:
			return 0;
		}

		return 0;
	}

	uint32_t TexUtil::get_glDataType_from_textureFormat(const graphics::TextureFormat format, const bool isHalfFloat)
	{
		switch (format)
		{
		case mnemosy::graphics::MNSY_NONE:		return 0;

		case mnemosy::graphics::MNSY_R8:		return GL_UNSIGNED_BYTE;
		case mnemosy::graphics::MNSY_RG8:		return GL_UNSIGNED_BYTE;
		case mnemosy::graphics::MNSY_RGB8:		return GL_UNSIGNED_BYTE;
		case mnemosy::graphics::MNSY_RGBA8:		return GL_UNSIGNED_BYTE;

		case mnemosy::graphics::MNSY_R16:
			if (isHalfFloat) {
				return GL_HALF_FLOAT;
			}
			return GL_UNSIGNED_SHORT;
		case mnemosy::graphics::MNSY_RG16:
			if (isHalfFloat) {
				return GL_HALF_FLOAT;
			}
			return GL_UNSIGNED_SHORT;
		case mnemosy::graphics::MNSY_RGB16:
			if (isHalfFloat) {
				return GL_HALF_FLOAT;
			}
			return GL_UNSIGNED_SHORT;
		case mnemosy::graphics::MNSY_RGBA16:
			if (isHalfFloat) {
				return GL_HALF_FLOAT;
			}
			return GL_UNSIGNED_SHORT;

		case mnemosy::graphics::MNSY_R32:		return GL_FLOAT;
		case mnemosy::graphics::MNSY_RG32:		return GL_FLOAT;
		case mnemosy::graphics::MNSY_RGB32:		return GL_FLOAT;
		case mnemosy::graphics::MNSY_RGBA32:	return GL_FLOAT;

		default:
			return 0;
		}

		return 0;
	}

}