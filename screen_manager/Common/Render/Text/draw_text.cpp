#include "ppsspp_config.h"

#include "Common/System/Display.h"
#include "Common/GPU/thin3d.h"
#include "Common/Data/Hash/Hash.h"
#include "Common/Data/Text/WrapText.h"
#include "Common/Data/Encoding/Utf8.h"

#include "Common/Render/Text/draw_text.h"


SCREEN_TextDrawer::SCREEN_TextDrawer(SCREEN_Draw::SCREEN_DrawContext *draw) : draw_(draw) {
	// These probably shouldn't be state.
	dpiScale_ = CalculateDPIScale();
}
SCREEN_TextDrawer::~SCREEN_TextDrawer() {
}

float SCREEN_TextDrawerWordWrapper::MeasureWidth(const char *str, size_t bytes) {
	float w, h;
	drawer_->MeasureString(str, bytes, &w, &h);
	return w;
}

void SCREEN_TextDrawer::WrapString(std::string &out, const char *str, float maxW, int flags) {
	SCREEN_TextDrawerWordWrapper wrapper(this, str, maxW, flags);
	out = wrapper.Wrapped();
}

void SCREEN_TextDrawer::SetFontScale(float xscale, float yscale) {
	fontScaleX_ = xscale;
	fontScaleY_ = yscale;
}

float SCREEN_TextDrawer::CalculateDPIScale() {
	if (ignoreGlobalDpi_)
		return dpiScale_;
	float scale = g_dpi_scale_y;
	if (scale >= 1.0f) {
		scale = 1.0f;
	}
	return scale;
}

void SCREEN_TextDrawer::DrawStringRect(SCREEN_DrawBuffer &target, const char *str, const Bounds &bounds, uint32_t color, int align) {
	float x = bounds.x;
	float y = bounds.y;
	if (align & ALIGN_HCENTER) {
		x = bounds.centerX();
	} else if (align & ALIGN_RIGHT) {
		x = bounds.x2();
	}
	if (align & ALIGN_VCENTER) {
		y = bounds.centerY();
	} else if (align & ALIGN_BOTTOM) {
		y = bounds.y2();
	}

	std::string toDraw = str;
	int wrap = align & (FLAG_WRAP_TEXT | FLAG_ELLIPSIZE_TEXT);
	if (wrap) {
		bool rotated = (align & (ROTATE_90DEG_LEFT | ROTATE_90DEG_RIGHT)) != 0;
		WrapString(toDraw, str, rotated ? bounds.h : bounds.w, wrap);
	}

	DrawString(target, toDraw.c_str(), x, y, color, align);
}

void SCREEN_TextDrawer::DrawStringBitmapRect(std::vector<uint8_t> &bitmapData, TextStringEntry &entry, SCREEN_Draw::SCREEN_DataFormat texFormat, const char *str, const Bounds &bounds, int align) {
	std::string toDraw = str;
	int wrap = align & (FLAG_WRAP_TEXT | FLAG_ELLIPSIZE_TEXT);
	if (wrap) {
		bool rotated = (align & (ROTATE_90DEG_LEFT | ROTATE_90DEG_RIGHT)) != 0;
		WrapString(toDraw, str, rotated ? bounds.h : bounds.w, wrap);
	}

	DrawStringBitmap(bitmapData, entry, texFormat, toDraw.c_str(), align);
}

SCREEN_TextDrawer *SCREEN_TextDrawer::Create(SCREEN_Draw::SCREEN_DrawContext *draw) {
	SCREEN_TextDrawer *drawer = nullptr;

	if (drawer && !drawer->IsReady()) {
		delete drawer;
		drawer = nullptr;
	}
	return drawer;
}
