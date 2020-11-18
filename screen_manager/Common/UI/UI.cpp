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

SCREEN_DrawBuffer SCREEN_ui_draw2d;
SCREEN_DrawBuffer SCREEN_ui_draw2d_front;

void UIBegin(SCREEN_Draw::SCREEN_Pipeline *pipeline) {
	SCREEN_ui_draw2d.Begin(pipeline);
	SCREEN_ui_draw2d_front.Begin(pipeline);
}

void SCREEN_UIFlush() {
	SCREEN_ui_draw2d.Flush();
	SCREEN_ui_draw2d_front.Flush();
}
