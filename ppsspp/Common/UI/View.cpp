#include <algorithm>
#include <mutex>
#include <iostream>
#include "Common/Input/InputState.h"
#include "Common/Input/KeyCodes.h"
#include "Common/Render/DrawBuffer.h"
#include "Common/Render/TextureAtlas.h"
#include "Common/Data/Encoding/Utf8.h"
#include "Common/UI/UI.h"
#include "Common/UI/View.h"
#include "Common/UI/Context.h"
#include "Common/UI/Tween.h"
#include "Common/UI/Root.h"
#include "Common/GPU/thin3d.h"
#include "Common/System/System.h"
#include "Common/TimeUtil.h"
#include "Common/StringUtils.h"
#include "../screen_manager/UI/Scale.h"

namespace UI {

const float ITEM_HEIGHT = f64;
const float MIN_TEXT_SCALE = f0_88;
const float MAX_ITEM_SIZE = 65535.0f;

void MeasureBySpec(Size sz, float contentWidth, MeasureSpec spec, float *measured) {
	*measured = sz;
	if (sz == WRAP_CONTENT) {
		if (spec.type == UNSPECIFIED)
			*measured = contentWidth;
		else if (spec.type == AT_MOST)
			*measured = contentWidth < spec.size ? contentWidth : spec.size;
		else if (spec.type == EXACTLY)
			*measured = spec.size;
	} else if (sz == FILL_PARENT) {
		// UNSPECIFIED may have a minimum size of the parent.  Let's use it to fill.
		if (spec.type == UNSPECIFIED)
			*measured = std::max(spec.size, contentWidth);
		else
			*measured = spec.size;
	} else if (spec.type == EXACTLY || (spec.type == AT_MOST && *measured > spec.size)) {
		*measured = spec.size;
	}
}

void ApplyBoundBySpec(float &bound, MeasureSpec spec) {
	switch (spec.type) {
	case AT_MOST:
		bound = bound < spec.size ? bound : spec.size;
		break;
	case EXACTLY:
		bound = spec.size;
		break;
	case UNSPECIFIED:
		break;
	}
}

void ApplyBoundsBySpec(Bounds &bounds, MeasureSpec horiz, MeasureSpec vert) {
	ApplyBoundBySpec(bounds.w, horiz);
	ApplyBoundBySpec(bounds.h, vert);
}

void Event::Add(std::function<EventReturn(EventParams&)> func) {
	HandlerRegistration reg;
	reg.func = func;
	handlers_.push_back(reg);
}

// Call this from input thread or whatever, it doesn't matter
void Event::Trigger(EventParams &e) {
	EventTriggered(this, e);
}

// Call this from UI thread
EventReturn Event::Dispatch(EventParams &e) {
	for (auto iter = handlers_.begin(); iter != handlers_.end(); ++iter) {
		if ((iter->func)(e) == UI::EVENT_DONE) {
			// Event is handled, stop looping immediately. This event might even have gotten deleted.
			return UI::EVENT_DONE;
		}
	}
	return UI::EVENT_SKIPPED;
}

Event::~Event() {
	handlers_.clear();
	RemoveQueuedEventsByEvent(this);
}

View::~View() {
	if (HasFocus())
		SetFocusedView(0);
	RemoveQueuedEventsByView(this);

	// Could use unique_ptr, but then we have to include tween everywhere.
	for (auto &tween : tweens_)
		delete tween;
	tweens_.clear();
}

void View::Update() {
	for (size_t i = 0; i < tweens_.size(); ++i) {
		Tween *tween = tweens_[i];
		if (!tween->Finished()) {
			tween->Apply(this);
		} else if (!tween->Persists()) {
			tweens_.erase(tweens_.begin() + i);
			i--;
			delete tween;
		}
	}
}

void View::Measure(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert) {
	float contentW = 0.0f, contentH = 0.0f;
	GetContentDimensionsBySpec(dc, horiz, vert, contentW, contentH);
	MeasureBySpec(layoutParams_->width, contentW, horiz, &measuredWidth_);
	MeasureBySpec(layoutParams_->height, contentH, vert, &measuredHeight_);
}

// Default values

void View::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	w = f10;
	h = f10;
}

void View::GetContentDimensionsBySpec(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert, float &w, float &h) const {
	GetContentDimensions(dc, w, h);
}

void View::Query(float x, float y, std::vector<View *> &list) {
	if (bounds_.Contains(x, y)) {
		list.push_back(this);
	}
}

std::string View::Describe() const {
	return PStringFromFormat("%0.1f,%0.1f %0.1fx%0.1f", bounds_.x, bounds_.y, bounds_.w, bounds_.h);
}


void View::PersistData(PersistStatus status, std::string anonId, PersistMap &storage) {
	// Remember if this view was a focused view.
	std::string tag = Tag();
	if (tag.empty()) {
		tag = anonId;
	}

	const std::string focusedKey = "ViewFocused::" + tag;
	switch (status) {
	case UI::PERSIST_SAVE:
		if (HasFocus()) {
			storage[focusedKey].resize(1);
		}
		break;
	case UI::PERSIST_RESTORE:
		if (storage.find(focusedKey) != storage.end()) {
			SetFocus();
		}
		break;
	}

	for (int i = 0; i < (int)tweens_.size(); ++i) {
		tweens_[i]->PersistData(status, tag + "/" + StringFromInt(i), storage);
	}
}

Point View::GetFocusPosition(FocusDirection dir) {
	// The +2/-2 is some extra fudge factor to cover for views sitting right next to each other.
	// Distance zero yields strange results otherwise.
	switch (dir) {
	case FOCUS_LEFT: return Point(bounds_.x + 2, bounds_.centerY());
	case FOCUS_RIGHT: return Point(bounds_.x2() - 2, bounds_.centerY());
	case FOCUS_UP: return Point(bounds_.centerX(), bounds_.y + 2);
	case FOCUS_DOWN: return Point(bounds_.centerX(), bounds_.y2() - 2);

	default:
		return bounds_.Center();
	}
}

bool View::SetFocus() {
	if (IsFocusMovementEnabled()) {
		if (CanBeFocused()) {
			SetFocusedView(this);
			return true;
		}
	}
	return false;
}

Clickable::Clickable(LayoutParams *layoutParams)
	: View(layoutParams) {
	// We set the colors later once we have a UIContext.
	bgColor_ = AddTween(new CallbackColorTween(0.1f));
	bgColor_->Persist();
}

void Clickable::DrawBG(UIContext &dc, const Style &style) {
	if (style.background.type == DRAW_SOLID_COLOR) {
		if (time_now_d() - bgColorLast_ >= 0.25f) {
			bgColor_->Reset(style.background.color);
		} else {
			bgColor_->Divert(style.background.color, down_ ? 0.05f : 0.1f);
		}
		bgColorLast_ = time_now_d();

		dc.FillRect(Drawable(bgColor_->CurrentValue()), bounds_);
	} else {
		dc.FillRect(style.background, bounds_);
	}
}

void Clickable::Click() {
	UI::EventParams e{};
	e.v = this;
	OnClick.Trigger(e);
};

void Clickable::FocusChanged(int focusFlags) {
	if (focusFlags & FF_LOSTFOCUS) {
		down_ = false;
		dragging_ = false;
	}
}

void Clickable::Touch(const TouchInput &input) {
	if (!IsEnabled()) {
		dragging_ = false;
		down_ = false;
		return;
	}

	if (input.flags & TOUCH_DOWN) {
		if (bounds_.Contains(input.x, input.y)) {
			if (IsFocusMovementEnabled())
				SetFocusedView(this);
			dragging_ = true;
			down_ = true;
		} else {
			down_ = false;
			dragging_ = false;
		}
	} else if (input.flags & TOUCH_MOVE) {
		if (dragging_)
			down_ = bounds_.Contains(input.x, input.y);
	}
	if (input.flags & TOUCH_UP) {
		if ((input.flags & TOUCH_CANCEL) == 0 && dragging_ && bounds_.Contains(input.x, input.y)) {
			Click();
		}
		down_ = false;
		downCountDown_ = 0;
		dragging_ = false;
	}
}

static bool MatchesKeyDef(const std::vector<KeyDef> &defs, const KeyInput &key) {
	// In addition to the actual search, we need to do another search where we replace the device ID with "ANY".
	return
		std::find(defs.begin(), defs.end(), KeyDef(key.deviceId, key.keyCode)) != defs.end() ||
		std::find(defs.begin(), defs.end(), KeyDef(DEVICE_ID_ANY, key.keyCode)) != defs.end();
}

// TODO: O/X confirm preference for xperia play?

bool IsDPadKey(const KeyInput &key) {
	if (dpadKeys.empty()) {
		return key.keyCode >= NKCODE_DPAD_UP && key.keyCode <= NKCODE_DPAD_RIGHT;
	} else {
		return MatchesKeyDef(dpadKeys, key);
	}
}

bool IsAcceptKey(const KeyInput &key) {
	if (confirmKeys.empty()) {
		// This path is pretty much not used, confirmKeys should be set.
		// TODO: Get rid of this stuff?
		if (key.deviceId == DEVICE_ID_KEYBOARD) {
			return key.keyCode == NKCODE_SPACE || key.keyCode == NKCODE_ENTER || key.keyCode == NKCODE_Z;
		} else {
			return key.keyCode == NKCODE_BUTTON_A || key.keyCode == NKCODE_BUTTON_CROSS || key.keyCode == NKCODE_BUTTON_1 || key.keyCode == NKCODE_DPAD_CENTER;
		}
	} else {
		return MatchesKeyDef(confirmKeys, key);
	}
}

bool IsEscapeKey(const KeyInput &key) {
	if (cancelKeys.empty()) {
		// This path is pretty much not used, cancelKeys should be set.
		// TODO: Get rid of this stuff?
		if (key.deviceId == DEVICE_ID_KEYBOARD) {
			return key.keyCode == NKCODE_ESCAPE || key.keyCode == NKCODE_BACK;
		} else {
			return key.keyCode == NKCODE_BUTTON_CIRCLE || key.keyCode == NKCODE_BUTTON_B || key.keyCode == NKCODE_BUTTON_2;
		}
	} else {
		return MatchesKeyDef(cancelKeys, key);
	}
}

bool IsTabLeftKey(const KeyInput &key) {
	if (tabLeftKeys.empty()) {
		// This path is pretty much not used, tabLeftKeys should be set.
		// TODO: Get rid of this stuff?
		return key.keyCode == NKCODE_BUTTON_L1;
	} else {
		return MatchesKeyDef(tabLeftKeys, key);
	}
}

bool IsTabRightKey(const KeyInput &key) {
	if (tabRightKeys.empty()) {
		// This path is pretty much not used, tabRightKeys should be set.
		// TODO: Get rid of this stuff?
		return key.keyCode == NKCODE_BUTTON_R1;
	} else {
		return MatchesKeyDef(tabRightKeys, key);
	}
}

bool Clickable::Key(const KeyInput &key) {
	if (!HasFocus() && key.deviceId != DEVICE_ID_MOUSE) {
		down_ = false;
		return false;
	}
	// TODO: Replace most of Update with this.

	bool ret = false;
	if (key.flags & KEY_DOWN) {
		if (IsAcceptKey(key)) {
			down_ = true;
			ret = true;
		}
	}
	if (key.flags & KEY_UP) {
		if (IsAcceptKey(key)) {
			if (down_) {
				Click();
				down_ = false;
				ret = true;
			}
		} else if (IsEscapeKey(key)) {
			down_ = false;
		}
	}
	return ret;
}

void StickyChoice::Touch(const TouchInput &input) {
	dragging_ = false;
	if (!IsEnabled()) {
		down_ = false;
		return;
	}

	if (input.flags & TOUCH_DOWN) {
		if (bounds_.Contains(input.x, input.y)) {
			if (IsFocusMovementEnabled())
				SetFocusedView(this);
			down_ = true;
			Click();
		}
	}
}

bool StickyChoice::Key(const KeyInput &key) {
	if (!HasFocus()) {
		return false;
	}

	// TODO: Replace most of Update with this.
	if (key.flags & KEY_DOWN) {
		if (IsAcceptKey(key)) {
			down_ = true;
			UI::PlayUISound(UI::UISound::TOGGLE_ON);
			Click();
			return true;
		}
	}
	return false;
}

void StickyChoice::FocusChanged(int focusFlags) {
	// Override Clickable's FocusChanged to do nothing.
}

Item::Item(LayoutParams *layoutParams) : InertView(layoutParams) {
	if (!layoutParams) {
		layoutParams_->width = FILL_PARENT;
		layoutParams_->height = ITEM_HEIGHT;
	}
}

void Item::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	w = 0.0f;
	h = 0.0f;
}

void ClickableItem::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	w = 0.0f;
	h = ITEM_HEIGHT;
}

ClickableItem::ClickableItem(LayoutParams *layoutParams) : Clickable(layoutParams) {
	if (!layoutParams) {
		if (layoutParams_->width == WRAP_CONTENT)
			layoutParams_->width = FILL_PARENT;
	}
}

void ClickableItem::Draw(UIContext &dc) {
	Style style = dc.theme->itemStyle;

	if (HasFocus()) {
		style = dc.theme->itemFocusedStyle;
	}
	if (down_) {
		style = dc.theme->itemDownStyle;
	}

	DrawBG(dc, style);
}

void Choice::Click() {
	ClickableItem::Click();
	UI::PlayUISound(UI::UISound::CONFIRM);
}

void Choice::GetContentDimensionsBySpec(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert, float &w, float &h) const {
	if (atlasImage_.isValid()) {
		dc.Draw()->GetAtlas()->measureImage(atlasImage_, &w, &h);
        w = w * gScale;
        h = h * gScale;
	} else {
		const float paddingX = f12;
		float availWidth = horiz.size - paddingX * 2 - textPadding_.horiz();
		if (availWidth < 0.0f) {
			// Let it have as much space as it needs.
			availWidth = MAX_ITEM_SIZE;
		}
		float scale = CalculateTextScale(dc, availWidth);
		Bounds availBounds(0, 0, availWidth, vert.size);
		dc.MeasureTextRect(dc.theme->uiFont, scale, scale, text_.c_str(), (int)text_.size(), availBounds, &w, &h, FLAG_WRAP_TEXT);
	}
	w += f10;
	h += f4;
	h = std::max(h, ITEM_HEIGHT);
}

float Choice::CalculateTextScale(const UIContext &dc, float availWidth) const {
	float actualWidth, actualHeight;
	Bounds availBounds(0, 0, availWidth, bounds_.h);
	dc.MeasureTextRect(dc.theme->uiFont, 1.0f, 1.0f, text_.c_str(), (int)text_.size(), availBounds, &actualWidth, &actualHeight);
	if (actualWidth > availWidth) {
		return std::max(MIN_TEXT_SCALE, availWidth / actualWidth);
	}
	return f1;
}

void Choice::HighlightChanged(bool highlighted){
	highlighted_ = highlighted;
}

void Choice::Draw(UIContext &dc) {
	if (!IsSticky()) {
		ClickableItem::Draw(dc);
	} else {
		Style style = dc.theme->itemStyle;
		if (highlighted_) {
			style = dc.theme->itemHighlightedStyle;
		}
		if (down_) {
			style = dc.theme->itemDownStyle;
		}
		if (HasFocus()) {
			style = dc.theme->itemFocusedStyle;
		}

		DrawBG(dc, style);
	}

	Style style = dc.theme->itemStyle;
	if (!IsEnabled()) {
		style = dc.theme->itemDisabledStyle;
	}

	if (atlasImage_.isValid()) {
		dc.Draw()->DrawImage(atlasImage_, bounds_.centerX(), bounds_.centerY(), f1, style.fgColor, ALIGN_CENTER);
	} else {
		dc.SetFontStyle(dc.theme->uiFont);

		const float paddingX = f5;
		const float availWidth = bounds_.w - paddingX * 2 - textPadding_.horiz();
		float scale = CalculateTextScale(dc, availWidth);

		dc.SetFontScale(f1, f1);
		if (centered_) {
			dc.DrawTextRect(text_.c_str(), bounds_, style.fgColor, ALIGN_CENTER | FLAG_WRAP_TEXT);
		} else {
			if (iconImage_.isValid()) {
				dc.Draw()->DrawImage(iconImage_, bounds_.x2() - 32 - paddingX, bounds_.centerY(), f0_5, style.fgColor, ALIGN_CENTER);
			}

			Bounds textBounds(bounds_.x + paddingX + textPadding_.left, bounds_.y, availWidth, bounds_.h);
			dc.DrawTextRect(text_.c_str(), textBounds, style.fgColor, ALIGN_VCENTER | FLAG_WRAP_TEXT);
		}
		dc.SetFontScale(f1, f1);
	}

	if (selected_) {
		dc.Draw()->DrawImage(dc.theme->checkOn, bounds_.x2() - 40, bounds_.centerY(), f1, style.fgColor, ALIGN_CENTER);
	}
}

InfoItem::InfoItem(const std::string &text, const std::string &rightText, LayoutParams *layoutParams)
	: Item(layoutParams), text_(text), rightText_(rightText) {
	// We set the colors later once we have a UIContext.
	bgColor_ = AddTween(new CallbackColorTween(0.1f));
	bgColor_->Persist();
	fgColor_ = AddTween(new CallbackColorTween(0.1f));
	fgColor_->Persist();
}

void InfoItem::Draw(UIContext &dc) {
	Item::Draw(dc);

	UI::Style style = HasFocus() ? dc.theme->itemFocusedStyle : dc.theme->infoStyle;

	if (style.background.type == DRAW_SOLID_COLOR) {
		// For a smoother fade, using the same color with 0 alpha.
		if ((style.background.color & 0xFF000000) == 0)
			style.background.color = dc.theme->itemFocusedStyle.background.color & 0x00FFFFFF;
		bgColor_->Divert(style.background.color & 0x7fffffff);
		style.background.color = bgColor_->CurrentValue();
	}
	fgColor_->Divert(style.fgColor);
	style.fgColor = fgColor_->CurrentValue();

	dc.FillRect(style.background, bounds_);

	const float paddingX = f12;

	dc.SetFontStyle(dc.theme->uiFont);
	dc.DrawText(text_.c_str(), bounds_.x + paddingX, bounds_.centerY(), style.fgColor, ALIGN_VCENTER);
	dc.DrawText(rightText_.c_str(), bounds_.x2() - paddingX, bounds_.centerY(), style.fgColor, ALIGN_VCENTER | ALIGN_RIGHT);
// 	dc.Draw()->DrawImageStretch(dc.theme->whiteImage, bounds_.x, bounds_.y, bounds_.x2(), bounds_.y + 2, dc.theme->itemDownStyle.bgColor);
}

ItemHeader::ItemHeader(const std::string &text, LayoutParams *layoutParams)
	: Item(layoutParams), text_(text) {
	layoutParams_->width = FILL_PARENT;
	layoutParams_->height = f40;
}

void ItemHeader::Draw(UIContext &dc) {
	dc.SetFontStyle(dc.theme->uiFontSmall);
	dc.DrawText(text_.c_str(), bounds_.x + 4, bounds_.centerY(), dc.theme->headerStyle.fgColor, ALIGN_LEFT | ALIGN_VCENTER);
	dc.Draw()->DrawImageStretch(dc.theme->whiteImage, bounds_.x, bounds_.y2()-2, bounds_.x2(), bounds_.y2(), dc.theme->headerStyle.fgColor);
}

void ItemHeader::GetContentDimensionsBySpec(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert, float &w, float &h) const {
	Bounds bounds(0, 0, layoutParams_->width, layoutParams_->height);
	if (bounds.w < 0) {
		// If there's no size, let's grow as big as we want.
		bounds.w = horiz.size == 0 ? MAX_ITEM_SIZE : horiz.size;
	}
	if (bounds.h < 0) {
		bounds.h = vert.size == 0 ? MAX_ITEM_SIZE : vert.size;
	}
	ApplyBoundsBySpec(bounds, horiz, vert);
	dc.MeasureTextRect(dc.theme->uiFontSmall, f1, f1, text_.c_str(), (int)text_.length(), bounds, &w, &h, ALIGN_LEFT | ALIGN_VCENTER);
}

void PopupHeader::Draw(UIContext &dc) {
	const float paddingHorizontal = f12;
	const float availableWidth = bounds_.w - paddingHorizontal * 2;

	float tw, th;
	dc.SetFontStyle(dc.theme->uiFont);
	dc.MeasureText(dc.GetFontStyle(), f1, f1, text_.c_str(), &tw, &th, 0);

	float sineWidth = std::max(0.0f, (tw - availableWidth)) / 2.0f;

	float tx = paddingHorizontal;
	if (availableWidth < tw) {
		float overageRatio = 1.5f * availableWidth * 1.0f / tw;
		tx -= (1.0f + sin(time_now_d() * overageRatio)) * sineWidth;
		Bounds tb = bounds_;
		tb.x = bounds_.x + paddingHorizontal;
		tb.w = bounds_.w - paddingHorizontal * 2;
		dc.PushScissor(tb);
	}

	dc.DrawText(text_.c_str(), bounds_.x + tx, bounds_.centerY(), dc.theme->popupTitle.fgColor, ALIGN_LEFT | ALIGN_VCENTER);
	dc.Draw()->DrawImageStretch(dc.theme->whiteImage, bounds_.x, bounds_.y2()-2, bounds_.x2(), bounds_.y2(), dc.theme->popupTitle.fgColor);

	if (availableWidth < tw) {
		dc.PopScissor();
	}
}

void CheckBox::Toggle() {
	if (toggle_) {
		*toggle_ = !(*toggle_);
		UI::PlayUISound(*toggle_ ? UI::UISound::TOGGLE_ON : UI::UISound::TOGGLE_OFF);
	}
}

bool CheckBox::Toggled() const {
	if (toggle_)
		return *toggle_;
	return false;
}

EventReturn CheckBox::OnClicked(EventParams &e) {
	Toggle();
	return EVENT_CONTINUE;  // It's safe to keep processing events.
}

void CheckBox::Draw(UIContext &dc) {
	Style style = dc.theme->itemStyle;
	if (!IsEnabled())
		style = dc.theme->itemDisabledStyle;
	dc.SetFontStyle(dc.theme->uiFont);

	ClickableItem::Draw(dc);

	ImageID image = Toggled() ? dc.theme->checkOn : dc.theme->checkOff;
	float imageW, imageH;
	dc.Draw()->MeasureImage(image, &imageW, &imageH);

	const float paddingX = f12;
	// Padding right of the checkbox image too.
	const float availWidth = bounds_.w - paddingX * 2 - imageW - paddingX;
	float scale = CalculateTextScale(dc, availWidth);

	dc.SetFontScale(scale, scale);
	Bounds textBounds(bounds_.x + paddingX, bounds_.y, availWidth, bounds_.h);
	dc.DrawTextRect(text_.c_str(), textBounds, style.fgColor, ALIGN_VCENTER | FLAG_WRAP_TEXT);
	dc.Draw()->DrawImage(image, bounds_.x2() - paddingX, bounds_.centerY(), f1, style.fgColor, ALIGN_RIGHT | ALIGN_VCENTER);
	dc.SetFontScale(f1, f1);
}

float CheckBox::CalculateTextScale(const UIContext &dc, float availWidth) const {
	float actualWidth, actualHeight;
	Bounds availBounds(0, 0, availWidth, bounds_.h);
	dc.MeasureTextRect(dc.theme->uiFont, f1, f1, text_.c_str(), (int)text_.size(), availBounds, &actualWidth, &actualHeight, ALIGN_VCENTER);
	if (actualWidth > availWidth) {
		return std::max(MIN_TEXT_SCALE, availWidth / actualWidth);
	}
	return f1;
}

void CheckBox::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	ImageID image = Toggled() ? dc.theme->checkOn : dc.theme->checkOff;
	float imageW, imageH;
	dc.Draw()->MeasureImage(image, &imageW, &imageH);

	const float paddingX = f12;
	// Padding right of the checkbox image too.
	float availWidth = bounds_.w - paddingX * 2 - imageW - paddingX;
	if (availWidth < 0.0f) {
		// Let it have as much space as it needs.
		availWidth = MAX_ITEM_SIZE;
	}
	float scale = CalculateTextScale(dc, availWidth);

	float actualWidth, actualHeight;
	Bounds availBounds(0, 0, availWidth, bounds_.h);
	dc.MeasureTextRect(dc.theme->uiFont, scale, scale, text_.c_str(), (int)text_.size(), availBounds, &actualWidth, &actualHeight, ALIGN_VCENTER | FLAG_WRAP_TEXT);

	w = bounds_.w;
	h = std::max(actualHeight, ITEM_HEIGHT);
}

void BitCheckBox::Toggle() {
	if (bitfield_) {
		*bitfield_ = *bitfield_ ^ bit_;
		if (*bitfield_ & bit_) {
			UI::PlayUISound(UI::UISound::TOGGLE_ON);
		} else {
			UI::PlayUISound(UI::UISound::TOGGLE_OFF);
		}
	}
}

bool BitCheckBox::Toggled() const {
	if (bitfield_)
		return (bit_ & *bitfield_) == bit_;
	return false;
}

void Button::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	if (imageID_.isValid()) {
		dc.Draw()->GetAtlas()->measureImage(imageID_, &w, &h);
	} else {
		dc.MeasureText(dc.theme->uiFont, f1, f1, text_.c_str(), &w, &h);
	}
	// Add some internal padding to not look totally ugly
	w += paddingW_;
	h += paddingH_;

	w *= scale_;
	h *= scale_;
}

void Button::Click() {
	Clickable::Click();
	UI::PlayUISound(UI::UISound::CONFIRM);
}

void Button::Draw(UIContext &dc) {
	Style style = dc.theme->buttonStyle;

	if (HasFocus()) style = dc.theme->buttonFocusedStyle;
	if (down_) style = dc.theme->buttonDownStyle;
	if (!IsEnabled()) style = dc.theme->buttonDisabledStyle;

	// dc.Draw()->DrawImage4Grid(style.image, bounds_.x, bounds_.y, bounds_.x2(), bounds_.y2(), style.bgColor);
	DrawBG(dc, style);
	float tw, th;
	dc.MeasureText(dc.theme->uiFont, f1, f1, text_.c_str(), &tw, &th);
	tw *= scale_;
	th *= scale_;

	if (tw > bounds_.w || imageID_.isValid()) {
		dc.PushScissor(bounds_);
	}
	dc.SetFontStyle(dc.theme->uiFont);
	dc.SetFontScale(f1, f1);
	if (imageID_.isValid() && text_.empty()) {
		dc.Draw()->DrawImage(imageID_, bounds_.centerX(), bounds_.centerY(), scale_, 0xFFFFFFFF, ALIGN_CENTER);
	} else if (!text_.empty()) {
		dc.DrawText(text_.c_str(), bounds_.centerX(), bounds_.centerY(), style.fgColor, ALIGN_CENTER);
		if (imageID_.isValid()) {
			const AtlasImage *img = dc.Draw()->GetAtlas()->getImage(imageID_);
			if (img) {
				dc.Draw()->DrawImage(imageID_, bounds_.centerX() - tw / 2 - 5 - img->w / 2, bounds_.centerY(), f1, 0xFFFFFFFF, ALIGN_CENTER);
			}
		}
	}
	dc.SetFontScale(f1, f1);

	if (tw > bounds_.w || imageID_.isValid()) {
		dc.PopScissor();
	}
}

void ImageView::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	dc.Draw()->GetAtlas()->measureImage(atlasImage_, &w, &h);
	// TODO: involve sizemode
}

void ImageView::Draw(UIContext &dc) {
	const AtlasImage *img = dc.Draw()->GetAtlas()->getImage(atlasImage_);
	if (img) {
		// TODO: involve sizemode
		float scale = bounds_.w / img->w;
		dc.Draw()->DrawImage(atlasImage_, bounds_.x, bounds_.y, scale, 0xFFFFFFFF, ALIGN_TOPLEFT);
	}
}

void TextView::GetContentDimensionsBySpec(const UIContext &dc, MeasureSpec horiz, MeasureSpec vert, float &w, float &h) const {
	Bounds bounds(0, 0, layoutParams_->width, layoutParams_->height);
	if (bounds.w < 0) {
		// If there's no size, let's grow as big as we want.
		bounds.w = horiz.size == 0 ? MAX_ITEM_SIZE : horiz.size;
	}
	if (bounds.h < 0) {
		bounds.h = vert.size == 0 ? MAX_ITEM_SIZE : vert.size;
	}
	ApplyBoundsBySpec(bounds, horiz, vert);
	dc.MeasureTextRect(small_ ? dc.theme->uiFontSmall : dc.theme->uiFont, f1, f1, text_.c_str(), (int)text_.length(), bounds, &w, &h, textAlign_);
}

void TextView::Draw(UIContext &dc) {
	uint32_t textColor = hasTextColor_ ? textColor_ : dc.theme->infoStyle.fgColor;
	if (!(textColor & 0xFF000000))
		return;

	bool clip = false;
	if (measuredWidth_ > bounds_.w || measuredHeight_ > bounds_.h)
		clip = true;
	if (bounds_.w < 0 || bounds_.h < 0 || !clip_) {
		// We have a layout but, but try not to screw up rendering.
		// TODO: Fix properly.
		clip = false;
	}
	if (clip) {
		dc.Flush();
		dc.PushScissor(bounds_);
	}
	// In case it's been made focusable.
	if (HasFocus()) {
		UI::Style style = dc.theme->itemFocusedStyle;
		style.background.color &= 0x7fffffff;
		dc.FillRect(style.background, bounds_);
	}
	dc.SetFontStyle(small_ ? dc.theme->uiFontSmall : dc.theme->uiFont);
	dc.SetFontScale(f1, f1);
	if (shadow_) {
		uint32_t shadowColor = 0x80000000;
		dc.DrawTextRect(text_.c_str(), bounds_.Offset(f1, f1), shadowColor, textAlign_);
	}
	dc.DrawTextRect(text_.c_str(), bounds_, textColor, textAlign_);
	if (clip) {
		dc.PopScissor();
	}
}

TextEdit::TextEdit(const std::string &text, const std::string &placeholderText, LayoutParams *layoutParams)
  : View(layoutParams), text_(text), undo_(text), placeholderText_(placeholderText),
    textColor_(0xFFFFFFFF), maxLen_(255) {
	caret_ = (int)text_.size();
}

void TextEdit::Draw(UIContext &dc) {
	dc.PushScissor(bounds_);
	dc.SetFontStyle(dc.theme->uiFont);
	dc.FillRect(HasFocus() ? UI::Drawable(0x80000000) : UI::Drawable(0x30000000), bounds_);

	uint32_t textColor = hasTextColor_ ? textColor_ : dc.theme->infoStyle.fgColor;
	float textX = bounds_.x;
	float w, h;

	Bounds textBounds = bounds_;
	textBounds.x = textX - scrollPos_;

	if (text_.empty()) {
		if (placeholderText_.size()) {
			uint32_t c = textColor & 0x50FFFFFF;
			dc.DrawTextRect(placeholderText_.c_str(), bounds_, c, ALIGN_CENTER);
		}
	} else {
		dc.DrawTextRect(text_.c_str(), textBounds, textColor, ALIGN_VCENTER | ALIGN_LEFT | align_);
	}

	if (HasFocus()) {
		// Hack to find the caret position. Might want to find a better way...
		dc.MeasureTextCount(dc.theme->uiFont, f1, f1, text_.c_str(), caret_, &w, &h, ALIGN_VCENTER | ALIGN_LEFT | align_);
		float caretX = w - scrollPos_;
		if (caretX > bounds_.w) {
			scrollPos_ += caretX - bounds_.w;
		}
		if (caretX < 0) {
			scrollPos_ += caretX;
		}
		caretX += textX;
		dc.FillRect(UI::Drawable(textColor), Bounds(caretX - 1, bounds_.y + 2, 3, bounds_.h - 4));
	}
	dc.PopScissor();
}

void TextEdit::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	dc.MeasureText(dc.theme->uiFont, f1, f1, text_.size() ? text_.c_str() : "Wj", &w, &h, align_);
	w += f2;
	h += f2;
}

// Handles both windows and unix line endings.
static std::string FirstLine(const std::string &text) {
	size_t pos = text.find("\r\n");
	if (pos != std::string::npos) {
		return text.substr(0, pos);
	}
	pos = text.find('\n');
	if (pos != std::string::npos) {
		return text.substr(0, pos);
	}
	return text;
}

void TextEdit::Touch(const TouchInput &touch) {
	if (touch.flags & TOUCH_DOWN) {
		if (bounds_.Contains(touch.x, touch.y)) {
			SetFocusedView(this, true);
		}
	}
}

bool TextEdit::Key(const KeyInput &input) {
	if (!HasFocus())
		return false;
	bool textChanged = false;
	// Process hardcoded navigation keys. These aren't chars.
	if (input.flags & KEY_DOWN) {
		switch (input.keyCode) {
		case NKCODE_CTRL_LEFT:
		case NKCODE_CTRL_RIGHT:
			ctrlDown_ = true;
			break;
		case NKCODE_DPAD_LEFT:  // ASCII left arrow
			u8_dec(text_.c_str(), &caret_);
			break;
		case NKCODE_DPAD_RIGHT: // ASCII right arrow
			u8_inc(text_.c_str(), &caret_);
			break;
		case NKCODE_MOVE_HOME:
		case NKCODE_PAGE_UP:
			caret_ = 0;
			break;
		case NKCODE_MOVE_END:
		case NKCODE_PAGE_DOWN:
			caret_ = (int)text_.size();
			break;
		case NKCODE_FORWARD_DEL:
			if (caret_ < (int)text_.size()) {
				int endCaret = caret_;
				u8_inc(text_.c_str(), &endCaret);
				undo_ = text_;
				text_.erase(text_.begin() + caret_, text_.begin() + endCaret);
				textChanged = true;
			}
			break;
		case NKCODE_DEL:
			if (caret_ > 0) {
				int begCaret = caret_;
				u8_dec(text_.c_str(), &begCaret);
				undo_ = text_;
				text_.erase(text_.begin() + begCaret, text_.begin() + caret_);
				caret_--;
				textChanged = true;
			}
			break;
		case NKCODE_ENTER:
			{
				EventParams e{};
				e.v = this;
				e.s = text_;
				OnEnter.Trigger(e);
				break;
			}
		case NKCODE_BACK:
		case NKCODE_ESCAPE:
			return false;
		}

		if (ctrlDown_) {
			switch (input.keyCode) {
			case NKCODE_C:
				// Just copy the entire text contents, until we get selection support.
				System_SendMessage("setclipboardtext", text_.c_str());
				break;
			case NKCODE_V:
				{
					std::string clipText = System_GetProperty(SYSPROP_CLIPBOARD_TEXT);
					clipText = FirstLine(clipText);
					if (clipText.size()) {
						// Until we get selection, replace the whole text
						undo_ = text_;
						text_.clear();
						caret_ = 0;

						size_t maxPaste = maxLen_ - text_.size();
						if (clipText.size() > maxPaste) {
							int end = 0;
							while ((size_t)end < maxPaste) {
								u8_inc(clipText.c_str(), &end);
							}
							if (end > 0) {
								u8_dec(clipText.c_str(), &end);
							}
							clipText = clipText.substr(0, end);
						}
						InsertAtCaret(clipText.c_str());
						textChanged = true;
					}
				}
				break;
			case NKCODE_Z:
				text_ = undo_;
				break;
			}
		}

		if (caret_ < 0) {
			caret_ = 0;
		}
		if (caret_ > (int)text_.size()) {
			caret_ = (int)text_.size();
		}
	}

	if (input.flags & KEY_UP) {
		switch (input.keyCode) {
		case NKCODE_CTRL_LEFT:
		case NKCODE_CTRL_RIGHT:
			ctrlDown_ = false;
			break;
		}
	}

	// Process chars.
	if (input.flags & KEY_CHAR) {
		int unichar = input.keyCode;
		if (unichar >= 0x20 && !ctrlDown_) {  // Ignore control characters.
			// Insert it! (todo: do it with a string insert)
			char buf[8];
			buf[u8_wc_toutf8(buf, unichar)] = '\0';
			if (strlen(buf) + text_.size() < maxLen_) {
				undo_ = text_;
				InsertAtCaret(buf);
				textChanged = true;
			}
		}
	}

	if (textChanged) {
		UI::EventParams e{};
		e.v = this;
		OnTextChange.Trigger(e);
	}
	return true;
}

void TextEdit::InsertAtCaret(const char *text) {
	size_t len = strlen(text);
	for (size_t i = 0; i < len; i++) {
		text_.insert(text_.begin() + caret_, text[i]);
		caret_++;
	}
}

void ProgressBar::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	dc.MeasureText(dc.theme->uiFont, f1, f1, "  100%  ", &w, &h);
}

void ProgressBar::Draw(UIContext &dc) {
	char temp[32];
	sprintf(temp, "%i%%", (int)(progress_ * f100));
	dc.Draw()->DrawImageStretch(dc.theme->whiteImage, bounds_.x, bounds_.y, bounds_.x + bounds_.w * progress_, bounds_.y2(), 0xc0c0c0c0);
	dc.SetFontStyle(dc.theme->uiFont);
	dc.DrawTextRect(temp, bounds_, 0xFFFFFFFF, ALIGN_CENTER);
}

void Spinner::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	w = f48;
	h = f48;
}

void Spinner::Draw(UIContext &dc) {
	if (!(color_ & 0xFF000000))
		return;
	double t = time_now_d() * 1.3f;
	double angle = fmod(t, M_PI * 2.0);
	float r = bounds_.w * 0.5f;
	double da = M_PI * 2.0 / numImages_;
	for (int i = 0; i < numImages_; i++) {
		double a = angle + i * da;
		float x = (float)cos(a) * r;
		float y = (float)sin(a) * r;
		dc.Draw()->DrawImage(images_[i], bounds_.centerX() + x, bounds_.centerY() + y, f1, color_, ALIGN_CENTER);
	}
}

void TriggerButton::Touch(const TouchInput &input) {
	if (input.flags & TOUCH_DOWN) {
		if (bounds_.Contains(input.x, input.y)) {
			down_ |= 1 << input.id;
		}
	}
	if (input.flags & TOUCH_MOVE) {
		if (bounds_.Contains(input.x, input.y))
			down_ |= 1 << input.id;
		else
			down_ &= ~(1 << input.id);
	}

	if (input.flags & TOUCH_UP) {
		down_ &= ~(1 << input.id);
	}

	if (down_ != 0) {
		*bitField_ |= bit_;
	} else {
		*bitField_ &= ~bit_;
	}
}

void TriggerButton::Draw(UIContext &dc) {
	dc.Draw()->DrawImage(imageBackground_, bounds_.centerX(), bounds_.centerY(), f1, 0xFFFFFFFF, ALIGN_CENTER);
	dc.Draw()->DrawImage(imageForeground_, bounds_.centerX(), bounds_.centerY(), f1, 0xFFFFFFFF, ALIGN_CENTER);
}

void TriggerButton::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	dc.Draw()->GetAtlas()->measureImage(imageBackground_, &w, &h);
}

bool Slider::Key(const KeyInput &input) {
	if (HasFocus() && (input.flags & (KEY_DOWN | KEY_IS_REPEAT)) == KEY_DOWN) {
		if (ApplyKey(input.keyCode)) {
			Clamp();
			repeat_ = 0;
			repeatCode_ = input.keyCode;
			return true;
		}
		return false;
	} else if ((input.flags & KEY_UP) && input.keyCode == repeatCode_) {
		repeat_ = -1;
		return false;
	} else {
		return false;
	}
}

bool Slider::ApplyKey(int keyCode) {
	switch (keyCode) {
	case NKCODE_DPAD_LEFT:
	case NKCODE_MINUS:
	case NKCODE_NUMPAD_SUBTRACT:
		*value_ -= step_;
		break;
	case NKCODE_DPAD_RIGHT:
	case NKCODE_PLUS:
	case NKCODE_NUMPAD_ADD:
		*value_ += step_;
		break;
	case NKCODE_PAGE_UP:
		*value_ -= step_ * 10;
		break;
	case NKCODE_PAGE_DOWN:
		*value_ += step_ * 10;
		break;
	case NKCODE_MOVE_HOME:
		*value_ = minValue_;
		break;
	case NKCODE_MOVE_END:
		*value_ = maxValue_;
		break;
	default:
		return false;
	}
	return true;
}

void Slider::Touch(const TouchInput &input) {
	// Calling it afterwards, so dragging_ hasn't been set false yet when checking it above.
	Clickable::Touch(input);
	if (dragging_) {
		float relativeX = (input.x - (bounds_.x + paddingLeft_)) / (bounds_.w - paddingLeft_ - paddingRight_);
		*value_ = floorf(relativeX * (maxValue_ - minValue_) + minValue_ + 0.5f);
		Clamp();
		EventParams params{};
		params.v = this;
		params.a = (uint32_t)(*value_);
		params.f = (float)(*value_);
		OnChange.Trigger(params);
	}

	// Cancel any key repeat.
	repeat_ = -1;
}

void Slider::Clamp() {
	if (*value_ < minValue_) *value_ = minValue_;
	else if (*value_ > maxValue_) *value_ = maxValue_;

	// Clamp the value to be a multiple of the nearest step (e.g. if step == 5, value == 293, it'll round down to 290).
	*value_ = *value_ - fmodf(*value_, step_);
}

void Slider::Draw(UIContext &dc) {
	bool focus = HasFocus();
	uint32_t linecolor = dc.theme->popupTitle.fgColor;
	Style knobStyle = (down_ || focus) ? dc.theme->popupTitle : dc.theme->popupStyle;

	float knobX = ((float)(*value_) - minValue_) / (maxValue_ - minValue_) * (bounds_.w - paddingLeft_ - paddingRight_) + (bounds_.x + paddingLeft_);
	dc.FillRect(Drawable(linecolor), Bounds(bounds_.x + paddingLeft_, bounds_.centerY() - 2, knobX - (bounds_.x + paddingLeft_), 4));
	dc.FillRect(Drawable(0xFF808080), Bounds(knobX, bounds_.centerY() - 2, (bounds_.x + bounds_.w - paddingRight_ - knobX), 4));
	dc.Draw()->DrawImage(dc.theme->sliderKnob, knobX, bounds_.centerY(), f1, knobStyle.fgColor, ALIGN_CENTER);
	char temp[64];
	if (showPercent_)
		sprintf(temp, "%i%%", *value_);
	else
		sprintf(temp, "%i", *value_);
	dc.SetFontStyle(dc.theme->uiFont);
	dc.DrawText(temp, bounds_.x2() - 22, bounds_.centerY(), dc.theme->popupStyle.fgColor, ALIGN_CENTER | FLAG_DYNAMIC_ASCII);
}

void Slider::Update() {
	View::Update();
	if (repeat_ >= 0) {
		repeat_++;
	}

	if (repeat_ >= 47) {
		ApplyKey(repeatCode_);
		if ((maxValue_ - minValue_) / step_ >= 300) {
			ApplyKey(repeatCode_);
		}
		Clamp();
	} else if (repeat_ >= 12 && (repeat_ & 1) == 1) {
		ApplyKey(repeatCode_);
		Clamp();
	}
}

void Slider::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	// TODO
	w = f100;
	h = f50;
}

bool SliderFloat::Key(const KeyInput &input) {
	if (HasFocus() && (input.flags & (KEY_DOWN | KEY_IS_REPEAT)) == KEY_DOWN) {
		if (ApplyKey(input.keyCode)) {
			Clamp();
			repeat_ = 0;
			repeatCode_ = input.keyCode;
			return true;
		}
		return false;
	} else if ((input.flags & KEY_UP) && input.keyCode == repeatCode_) {
		repeat_ = -1;
		return false;
	} else {
		return false;
	}
}

bool SliderFloat::ApplyKey(int keyCode) {
	switch (keyCode) {
	case NKCODE_DPAD_LEFT:
	case NKCODE_MINUS:
	case NKCODE_NUMPAD_SUBTRACT:
		*value_ -= (maxValue_ - minValue_) / 50.0f;
		break;
	case NKCODE_DPAD_RIGHT:
	case NKCODE_PLUS:
	case NKCODE_NUMPAD_ADD:
		*value_ += (maxValue_ - minValue_) / 50.0f;
		break;
	case NKCODE_PAGE_UP:
		*value_ -= (maxValue_ - minValue_) / 5.0f;
		break;
	case NKCODE_PAGE_DOWN:
		*value_ += (maxValue_ - minValue_) / 5.0f;
		break;
	case NKCODE_MOVE_HOME:
		*value_ = minValue_;
		break;
	case NKCODE_MOVE_END:
		*value_ = maxValue_;
		break;
	default:
		return false;
	}
	return true;
}

void SliderFloat::Touch(const TouchInput &input) {
	Clickable::Touch(input);
	if (dragging_) {
		float relativeX = (input.x - (bounds_.x + paddingLeft_)) / (bounds_.w - paddingLeft_ - paddingRight_);
		*value_ = (relativeX * (maxValue_ - minValue_) + minValue_);
		Clamp();
		EventParams params{};
		params.v = this;
		params.a = (uint32_t)(*value_);
		params.f = (float)(*value_);
		OnChange.Trigger(params);
	}

	// Cancel any key repeat.
	repeat_ = -1;
}

void SliderFloat::Clamp() {
	if (*value_ < minValue_)
		*value_ = minValue_;
	else if (*value_ > maxValue_)
		*value_ = maxValue_;
}

void SliderFloat::Draw(UIContext &dc) {
	bool focus = HasFocus();
	uint32_t linecolor = dc.theme->popupTitle.fgColor;
	Style knobStyle = (down_ || focus) ? dc.theme->popupTitle : dc.theme->popupStyle;

	float knobX = (*value_ - minValue_) / (maxValue_ - minValue_) * (bounds_.w - paddingLeft_ - paddingRight_) + (bounds_.x + paddingLeft_);
	dc.FillRect(Drawable(linecolor), Bounds(bounds_.x + paddingLeft_, bounds_.centerY() - 2, knobX - (bounds_.x + paddingLeft_), 4));
	dc.FillRect(Drawable(0xFF808080), Bounds(knobX, bounds_.centerY() - 2, (bounds_.x + bounds_.w - paddingRight_ - knobX), 4));
	dc.Draw()->DrawImage(dc.theme->sliderKnob, knobX, bounds_.centerY(), f1, knobStyle.fgColor, ALIGN_CENTER);
	char temp[64];
	sprintf(temp, "%0.2f", *value_);
	dc.SetFontStyle(dc.theme->uiFont);
	dc.DrawText(temp, bounds_.x2() - 22, bounds_.centerY(), dc.theme->popupStyle.fgColor, ALIGN_CENTER);
}

void SliderFloat::Update() {
	View::Update();
	if (repeat_ >= 0) {
		repeat_++;
	}

	if (repeat_ >= 47) {
		ApplyKey(repeatCode_);
		Clamp();
	} else if (repeat_ >= 12 && (repeat_ & 1) == 1) {
		ApplyKey(repeatCode_);
		Clamp();
	}
}

void SliderFloat::GetContentDimensions(const UIContext &dc, float &w, float &h) const {
	// TODO
	w = f100;
	h = f50;
}

}  // namespace
