#pragma once

#include <string>
#include <vector>
#include "Common/GPU/thin3d.h"

class SCREEN_VulkanContext;

namespace SCREEN_Draw {

SCREEN_DrawContext *T3DCreateGLContext();

SCREEN_DrawContext *T3DCreateVulkanContext(SCREEN_VulkanContext *context, bool splitSubmit);

}  // namespace SCREEN_Draw
