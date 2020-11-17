// draw_text

// Uses system fonts to draw text. 
// Platform support will be added over time, initially just Win32.

// Caches strings in individual textures. Might later combine them into a big one
// with dynamically allocated space but too much trouble for now.

#pragma once

#include "ppsspp_config.h"

#include <memory>
#include <cstdint>

#include "Common/Data/Text/WrapText.h"
#include "Common/Render/DrawBuffer.h"

namespace SCREEN_Draw {
	class SCREEN_DrawContext;
	class SCREEN_Texture;
}

struct TextStringEntry {
	SCREEN_Draw::SCREEN_Texture *texture;
	int width;
	int height;
	int bmWidth;
	int bmHeight;
	int lastUsedFrame;
};

struct TextMeasureEntry {
	int width;
	int height;
	int lastUsedFrame;
};

class SCREEN_TextDrawer {
public:
	virtual ~SCREEN_TextDrawer();

	virtual bool IsReady() const { return true; }
	virtual uint32_t SetFont(const char *fontName, int size, int flags) = 0;
	virtual void SetFont(uint32_t fontHandle) = 0;  // Shortcut once you've set the font once.
	void SetFontScale(float xscale, float yscale);
	virtual void MeasureString(const char *str, size_t len, float *w, float *h) = 0;
	virtual void MeasureStringRect(const char *str, size_t len, const Bounds &bounds, float *w, float *h, int align = ALIGN_TOPLEFT) = 0;
	virtual void DrawString(SCREEN_DrawBuffer &target, const char *str, float x, float y, uint32_t color, int align = ALIGN_TOPLEFT) = 0;
	void DrawStringRect(SCREEN_DrawBuffer &target, const char *str, const Bounds &bounds, uint32_t color, int align);
	virtual void DrawStringBitmap(std::vector<uint8_t> &bitmapData, TextStringEntry &entry, SCREEN_Draw::SCREEN_DataFormat texFormat, const char *str, int align = ALIGN_TOPLEFT) = 0;
	void DrawStringBitmapRect(std::vector<uint8_t> &bitmapData, TextStringEntry &entry, SCREEN_Draw::SCREEN_DataFormat texFormat, const char *str, const Bounds &bounds, int align);
	// Use for housekeeping like throwing out old strings.
	virtual void OncePerFrame() = 0;

	float CalculateDPIScale();
	void SetForcedDPIScale(float dpi) {
		dpiScale_ = dpi;
		ignoreGlobalDpi_ = true;
	}

	// Factory function that selects implementation.
	static SCREEN_TextDrawer *Create(SCREEN_Draw::SCREEN_DrawContext *draw);

protected:
	SCREEN_TextDrawer(SCREEN_Draw::SCREEN_DrawContext *draw);

	SCREEN_Draw::SCREEN_DrawContext *draw_;
	virtual void ClearCache() = 0;
	void WrapString(std::string &out, const char *str, float maxWidth, int flags);

	struct CacheKey {
		bool operator < (const CacheKey &other) const {
			if (fontHash < other.fontHash)
				return true;
			if (fontHash > other.fontHash)
				return false;
			return text < other.text;
		}
		std::string text;
		uint32_t fontHash;
	};

	int frameCount_ = 0;
	float fontScaleX_ = 1.0f;
	float fontScaleY_ = 1.0f;
	float dpiScale_ = 1.0f;
	bool ignoreGlobalDpi_ = false;
};

class SCREEN_TextDrawerWordWrapper : public SCREEN_WordWrapper {
public:
	SCREEN_TextDrawerWordWrapper(SCREEN_TextDrawer *drawer, const char *str, float maxW, int flags)
		: SCREEN_WordWrapper(str, maxW, flags), drawer_(drawer) {}

protected:
	float MeasureWidth(const char *str, size_t bytes) override;

	SCREEN_TextDrawer *drawer_;
};
