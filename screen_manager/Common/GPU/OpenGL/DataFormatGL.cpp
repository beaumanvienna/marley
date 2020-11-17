#include "Common/GPU/OpenGL/DataFormatGL.h"
#include "Common/Log.h"

namespace SCREEN_Draw {

bool Thin3DFormatToFormatAndType(SCREEN_DataFormat fmt, GLuint &internalFormat, GLuint &format, GLuint &type, int &alignment) {
	alignment = 4;
	switch (fmt) {
	case SCREEN_DataFormat::R8G8B8A8_UNORM:
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		break;

	case SCREEN_DataFormat::D32F:
		internalFormat = GL_DEPTH_COMPONENT;
		format = GL_DEPTH_COMPONENT;
		type = GL_FLOAT;
		break;

#ifndef USING_GLES2
	case SCREEN_DataFormat::S8:
		internalFormat = GL_STENCIL_INDEX;
		format = GL_STENCIL_INDEX;
		type = GL_UNSIGNED_BYTE;
		alignment = 1;
		break;
#endif

	case SCREEN_DataFormat::R8G8B8_UNORM:
		internalFormat = GL_RGB;
		format = GL_RGB;
		type = GL_UNSIGNED_BYTE;
		alignment = 1;
		break;

	case SCREEN_DataFormat::R4G4B4A4_UNORM_PACK16:
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_UNSIGNED_SHORT_4_4_4_4;
		alignment = 2;
		break;

	case SCREEN_DataFormat::R5G6B5_UNORM_PACK16:
		internalFormat = GL_RGB;
		format = GL_RGB;
		type = GL_UNSIGNED_SHORT_5_6_5;
		alignment = 2;
		break;

	case SCREEN_DataFormat::R5G5B5A1_UNORM_PACK16:
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_UNSIGNED_SHORT_5_5_5_1;
		alignment = 2;
		break;

	case SCREEN_DataFormat::R32G32B32A32_FLOAT:
		internalFormat = GL_RGBA32F;
		format = GL_RGBA;
		type = GL_FLOAT;
		alignment = 16;
		break;

	default:
		return false;
	}
	return true;
}

}
