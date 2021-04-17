#include "ppsspp_config.h"

#include <algorithm>

#include "Common/System/Display.h"
#include "Common/System/System.h"
#include "Common/UI/UI.h"
#include "Common/UI/View.h"
#include "Common/UI/Context.h"
#include "Common/Render/DrawBuffer.h"
#include "Common/Render/Text/draw_text.h"
#include "../screen_manager/UI/Scale.h"

#include "Common/Log.h"
#include "UI/TextureUtil.h"

UIContext::UIContext() {
	fontStyle_ = new UI::FontStyle();
	bounds_ = Bounds(0, 0, dp_xres, dp_yres);
}

UIContext::~UIContext() {
	sampler_->Release();
	delete fontStyle_;
	delete textDrawer_;
}

void UIContext::Init(Draw::DrawContext *thin3d, Draw::Pipeline *uipipe, Draw::Pipeline *uipipenotex, DrawBuffer *uidrawbuffer, DrawBuffer *uidrawbufferTop) {
	using namespace Draw;
	draw_ = thin3d;
	sampler_ = draw_->CreateSamplerState({ TextureFilter::LINEAR, TextureFilter::LINEAR, TextureFilter::LINEAR });
	ui_pipeline_ = uipipe;
	ui_pipeline_notex_ = uipipenotex;
	uidrawbuffer_ = uidrawbuffer;
	uidrawbufferTop_ = uidrawbufferTop;
	textDrawer_ = TextDrawer::Create(thin3d);  // May return nullptr if no implementation is available for this platform.
}

void UIContext::BeginFrame() {
	if (!uitexture_) {
		uitexture_ = CreateTextureFromFile(draw_, "ui_atlas.zim", ImageFileType::ZIM, false);
		_dbg_assert_msg_(uitexture_, "Failed to load ui_atlas.zim.\n\nPlace it in the directory \"assets\" under your PPSSPP directory.");
	}
	uidrawbufferTop_->SetCurZ(0.0f);
	uidrawbuffer_->SetCurZ(0.0f);
	ActivateTopScissor();
}

void UIContext::Begin() {
	BeginPipeline(ui_pipeline_, sampler_);
}

void UIContext::BeginNoTex() {
	draw_->BindSamplerStates(0, 1, &sampler_);
	UIBegin(ui_pipeline_notex_);
}

void UIContext::BeginPipeline(Draw::Pipeline *pipeline, Draw::SamplerState *samplerState) {
	draw_->BindSamplerStates(0, 1, &samplerState);
	RebindTexture();
	UIBegin(pipeline);
}

void UIContext::RebindTexture() const {
	if (uitexture_)
		draw_->BindTexture(0, uitexture_->GetTexture());
}

void UIContext::Flush() {
	if (uidrawbuffer_) {
		uidrawbuffer_->Flush();
	}
	if (uidrawbufferTop_) {
		uidrawbufferTop_->Flush();
	}
}

void UIContext::SetCurZ(float curZ) {
	ui_draw2d.SetCurZ(curZ);
	ui_draw2d_front.SetCurZ(curZ);
}

// TODO: Support transformed bounds using stencil instead.
void UIContext::PushScissor(const Bounds &bounds) {
	Flush();
	Bounds clipped = TransformBounds(bounds);
	if (scissorStack_.size())
		clipped.Clip(scissorStack_.back());
	else
		clipped.Clip(bounds_);
	scissorStack_.push_back(clipped);
	ActivateTopScissor();
}

void UIContext::PopScissor() {
	Flush();
	scissorStack_.pop_back();
	ActivateTopScissor();
}

Bounds UIContext::GetScissorBounds() {
	if (!scissorStack_.empty())
		return scissorStack_.back();
	else
		return bounds_;
}

Bounds UIContext::GetLayoutBounds() const {
	Bounds bounds = GetBounds();

	float left = System_GetPropertyFloat(SYSPROP_DISPLAY_SAFE_INSET_LEFT);
	float right = System_GetPropertyFloat(SYSPROP_DISPLAY_SAFE_INSET_RIGHT);
	float top = System_GetPropertyFloat(SYSPROP_DISPLAY_SAFE_INSET_TOP);
	float bottom = System_GetPropertyFloat(SYSPROP_DISPLAY_SAFE_INSET_BOTTOM);

	// ILOG("Insets: %f %f %f %f", left, right, top, bottom);

	// Adjust left edge to compensate for cutouts (notches) if any.
	bounds.x += left;
	bounds.w -= (left + right);
	bounds.y += top;
	bounds.h -= (top + bottom);

	return bounds;
}

void UIContext::ActivateTopScissor() {
	Bounds bounds;
	if (scissorStack_.size()) {
		float scale_x = pixel_in_dps_x;
		float scale_y = pixel_in_dps_y;
		bounds = scissorStack_.back();
		int x = floorf(scale_x * bounds.x);
		int y = floorf(scale_y * bounds.y);
		int w = std::max(0.0f, ceilf(scale_x * bounds.w));
		int h = std::max(0.0f, ceilf(scale_y * bounds.h));
		draw_->SetScissorRect(x, y, w, h);
	} else {
		// Avoid rounding errors
		draw_->SetScissorRect(0, 0, pixel_xres, pixel_yres);
	}
}

void UIContext::SetFontScale(float scaleX, float scaleY) {
	fontScaleX_ = scaleX;
	fontScaleY_ = scaleY;
}

void UIContext::SetFontStyle(const UI::FontStyle &fontStyle) {
	*fontStyle_ = fontStyle;
	if (textDrawer_) {
		textDrawer_->SetFontScale(fontScaleX_, fontScaleY_);
		textDrawer_->SetFont(fontStyle.fontName.c_str(), fontStyle.sizePts, fontStyle.flags);
	}
}

void UIContext::MeasureText(const UI::FontStyle &style, float scaleX, float scaleY, const char *str, float *x, float *y, int align) const {
	MeasureTextCount(style, scaleX, scaleY, str, (int)strlen(str), x, y, align);
}

void UIContext::MeasureTextCount(const UI::FontStyle &style, float scaleX, float scaleY, const char *str, int count, float *x, float *y, int align) const {
	if (!textDrawer_ || (align & FLAG_DYNAMIC_ASCII)) {
		float sizeFactor = gScale*(float)style.sizePts / f24;
		Draw()->SetFontScale(scaleX * sizeFactor, scaleY * sizeFactor);
		Draw()->MeasureTextCount(style.atlasFont, str, count, x, y);
	} else {
		textDrawer_->SetFont(style.fontName.c_str(), style.sizePts, style.flags);
		textDrawer_->SetFontScale(scaleX*gScale, scaleY*gScale);
		textDrawer_->MeasureString(str, count, x, y);
		textDrawer_->SetFont(fontStyle_->fontName.c_str(), fontStyle_->sizePts, fontStyle_->flags);
	}
}

void UIContext::MeasureTextRect(const UI::FontStyle &style, float scaleX, float scaleY, const char *str, int count, const Bounds &bounds, float *x, float *y, int align) const {
	if (!textDrawer_ || (align & FLAG_DYNAMIC_ASCII)) {
		float sizeFactor = gScale*(float)style.sizePts / f24;
		Draw()->SetFontScale(scaleX * sizeFactor, scaleY * sizeFactor);
		Draw()->MeasureTextRect(style.atlasFont, str, count, bounds, x, y, align);
	} else {
		textDrawer_->SetFont(style.fontName.c_str(), style.sizePts, style.flags);
		textDrawer_->SetFontScale(scaleX*gScale, scaleY*gScale);
		textDrawer_->MeasureStringRect(str, count, bounds, x, y, align);
		textDrawer_->SetFont(fontStyle_->fontName.c_str(), fontStyle_->sizePts, fontStyle_->flags);
	}
}

void UIContext::DrawText(const char *str, float x, float y, uint32_t color, int align) {
	if (!textDrawer_ || (align & FLAG_DYNAMIC_ASCII)) {
		float sizeFactor = gScale*(float)fontStyle_->sizePts / f24;
		Draw()->SetFontScale(fontScaleX_ * sizeFactor, fontScaleY_ * sizeFactor);
		Draw()->DrawText(fontStyle_->atlasFont, str, x, y, color, align);
	} else {
		textDrawer_->SetFontScale(fontScaleX_*gScale, fontScaleY_*gScale);
		textDrawer_->DrawString(*Draw(), str, x, y, color, align);
		RebindTexture();
	}
}

void UIContext::DrawTextShadow(const char *str, float x, float y, uint32_t color, int align) {
	uint32_t alpha = (color >> 1) & 0xFF000000;
	DrawText(str, x + 2, y + 2, alpha, align);
	DrawText(str, x, y, color, align);
}

void UIContext::DrawTextRect(const char *str, const Bounds &bounds, uint32_t color, int align) {
	if (!textDrawer_ || (align & FLAG_DYNAMIC_ASCII)) {
		float sizeFactor = gScale * (float)fontStyle_->sizePts / f24;
		Draw()->SetFontScale(fontScaleX_ * sizeFactor, fontScaleY_ * sizeFactor);
		Draw()->DrawTextRect(fontStyle_->atlasFont, str, bounds.x, bounds.y, bounds.w, bounds.h, color, align);
	} else {
		textDrawer_->SetFontScale(fontScaleX_*gScale, fontScaleY_*gScale);
		Bounds rounded = bounds;
		rounded.x = floorf(rounded.x);
		rounded.y = floorf(rounded.y);
		textDrawer_->DrawStringRect(*Draw(), str, rounded, color, align);
		RebindTexture();
	}
}

void UIContext::FillRect(const UI::Drawable &drawable, const Bounds &bounds) {
	// Only draw if alpha is non-zero.
	if ((drawable.color & 0xFF000000) == 0)
		return;

	switch (drawable.type) {
	case UI::DRAW_SOLID_COLOR:
		uidrawbuffer_->DrawImageStretch(theme->whiteImage, bounds.x, bounds.y, bounds.x2(), bounds.y2(), drawable.color);
		break;
	case UI::DRAW_4GRID:
		uidrawbuffer_->DrawImage4Grid(drawable.image, bounds.x, bounds.y, bounds.x2(), bounds.y2(), drawable.color);
		break;
	case UI::DRAW_STRETCH_IMAGE:
		uidrawbuffer_->DrawImageStretch(drawable.image, bounds.x, bounds.y, bounds.x2(), bounds.y2(), drawable.color);
		break;
	case UI::DRAW_NOTHING:
		break;
	} 
}

void UIContext::PushTransform(const UITransform &transform) {
	Flush();

	using namespace Lin;

	Matrix4x4 m = Draw()->GetDrawMatrix();
	const Vec3 &t = transform.translate;
	Vec3 scaledTranslate = Vec3(
		t.x * m.xx + t.y * m.xy + t.z * m.xz + m.xw,
		t.x * m.yx + t.y * m.yy + t.z * m.yz + m.yw,
		t.x * m.zx + t.y * m.zy + t.z * m.zz + m.zw);

	m.translateAndScale(scaledTranslate, transform.scale);
	Draw()->PushDrawMatrix(m);
	Draw()->PushAlpha(transform.alpha);

	transformStack_.push_back(transform);
}

void UIContext::PopTransform() {
	Flush();

	transformStack_.pop_back();

	Draw()->PopDrawMatrix();
	Draw()->PopAlpha();
}

Bounds UIContext::TransformBounds(const Bounds &bounds) {
	if (!transformStack_.empty()) {
		const UITransform t = transformStack_.back();
		Bounds translated = bounds.Offset(t.translate.x, t.translate.y);

		// Scale around the center as the origin.
		float scaledX = (translated.x - dp_xres * 0.5f) * t.scale.x + dp_xres * 0.5f;
		float scaledY = (translated.y - dp_yres * 0.5f) * t.scale.y + dp_yres * 0.5f;

		return Bounds(scaledX, scaledY, translated.w * t.scale.x, translated.h * t.scale.y);
	}

	return bounds;
}
