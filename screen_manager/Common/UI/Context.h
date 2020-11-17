#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include "Common/Math/geom2d.h"
#include "Common/Math/lin/vec3.h"
#include "Common/Render/TextureAtlas.h"

// Everything you need to draw a UI collected into a single unit that can be passed around.
// Everything forward declared so this header is safe everywhere.

namespace SCREEN_Draw {
	class SCREEN_DrawContext;
	class SCREEN_Pipeline;
	class DepthStencilState;
	class SCREEN_Texture;
	class BlendState;
	class SCREEN_SamplerState;
	class RasterState;
}

class SCREEN_Texture;
class SCREEN_ManagedTexture;
class SCREEN_DrawBuffer;
class SCREEN_TextDrawer;

namespace SCREEN_UI {
	struct Drawable;
	struct EventParams;
	struct Theme;
	struct FontStyle;
	class Event;
	class View;
}

class SCREEN_DrawBuffer;

struct UITransform {
	// TODO: Or just use a matrix?
	SCREEN_Lin::SCREEN_Vec3 translate;
	SCREEN_Lin::SCREEN_Vec3 scale;
	float alpha;
};

class SCREEN_UIContext {
public:
	SCREEN_UIContext();
	~SCREEN_UIContext();

	void Init(SCREEN_Draw::SCREEN_DrawContext *thin3d, SCREEN_Draw::SCREEN_Pipeline *uipipe, SCREEN_Draw::SCREEN_Pipeline *uipipenotex, SCREEN_DrawBuffer *uidrawbuffer, SCREEN_DrawBuffer *uidrawbufferTop);

	void BeginFrame();

	void Begin();
	void BeginNoTex();
	void BeginPipeline(SCREEN_Draw::SCREEN_Pipeline *pipeline, SCREEN_Draw::SCREEN_SamplerState *samplerState);
	void Flush();

	void RebindTexture() const;

	// TODO: Support transformed bounds using stencil
	void PushScissor(const Bounds &bounds);
	void PopScissor();
	Bounds GetScissorBounds();

	void ActivateTopScissor();

	SCREEN_DrawBuffer *Draw() const { return uidrawbuffer_; }
	SCREEN_DrawBuffer *DrawTop() const { return uidrawbufferTop_; }
	const SCREEN_UI::Theme *theme;

	// Utility methods

	SCREEN_TextDrawer *Text() const { return textDrawer_; }

	void SetFontStyle(const SCREEN_UI::FontStyle &style);
	const SCREEN_UI::FontStyle &GetFontStyle() { return *fontStyle_; }
	void SetFontScale(float scaleX, float scaleY);
	void MeasureTextCount(const SCREEN_UI::FontStyle &style, float scaleX, float scaleY, const char *str, int count, float *x, float *y, int align = 0) const;
	void MeasureText(const SCREEN_UI::FontStyle &style, float scaleX, float scaleY, const char *str, float *x, float *y, int align = 0) const;
	void MeasureTextRect(const SCREEN_UI::FontStyle &style, float scaleX, float scaleY, const char *str, int count, const Bounds &bounds, float *x, float *y, int align = 0) const;
	void DrawText(const char *str, float x, float y, uint32_t color, int align = 0);
	void DrawTextShadow(const char *str, float x, float y, uint32_t color, int align = 0);
	void DrawTextRect(const char *str, const Bounds &bounds, uint32_t color, int align = 0);
	void FillRect(const SCREEN_UI::Drawable &drawable, const Bounds &bounds);

	// in dps, like dp_xres and dp_yres
	void SetBounds(const Bounds &b) { bounds_ = b; }
	const Bounds &GetBounds() const { return bounds_; }
	Bounds GetLayoutBounds() const;
	SCREEN_Draw::SCREEN_DrawContext *GetSCREEN_DrawContext() { return draw_; }
	void SetCurZ(float curZ);

	void PushTransform(const UITransform &transform);
	void PopTransform();
	Bounds TransformBounds(const Bounds &bounds);

private:
	SCREEN_Draw::SCREEN_DrawContext *draw_;
	Bounds bounds_;

	float fontScaleX_ = 1.0f;
	float fontScaleY_ = 1.0f;
	SCREEN_UI::FontStyle *fontStyle_ = nullptr;
	SCREEN_TextDrawer *textDrawer_ = nullptr;

	SCREEN_Draw::SCREEN_SamplerState *sampler_;
	SCREEN_Draw::SCREEN_Pipeline *ui_pipeline_ = nullptr;
	SCREEN_Draw::SCREEN_Pipeline *ui_pipeline_notex_ = nullptr;
	std::unique_ptr<SCREEN_ManagedTexture> uitexture_;

	SCREEN_DrawBuffer *uidrawbuffer_ = nullptr;
	SCREEN_DrawBuffer *uidrawbufferTop_ = nullptr;

	std::vector<Bounds> scissorStack_;
	std::vector<UITransform> transformStack_;
};
