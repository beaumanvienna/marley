// See header for documentation.

#include <string>
#include <vector>
#include <cmath>
#include <cstring>

#include "Common/Data/Color/RGBAUtil.h"
#include "Common/UI/UI.h"
#include "Common/UI/Context.h"
#include "Common/Render/TextureAtlas.h"
#include "Common/Render/DrawBuffer.h"

SCREEN_DrawBuffer ui_draw2d;
SCREEN_DrawBuffer ui_draw2d_front;

void UIBegin(SCREEN_Draw::SCREEN_Pipeline *pipeline) {
	ui_draw2d.Begin(pipeline);
	ui_draw2d_front.Begin(pipeline);
}

void UIFlush() {
	ui_draw2d.Flush();
	ui_draw2d_front.Flush();
}
