#pragma once

#include "Common/GPU/OpenGL/GLCommon.h"
#include "Common/GPU/DataFormat.h"

namespace SCREEN_Draw {

bool Thin3DFormatToFormatAndType(SCREEN_DataFormat fmt, GLuint &internalFormat, GLuint &format, GLuint &type, int &alignment);

}
