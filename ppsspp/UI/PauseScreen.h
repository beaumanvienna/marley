// Copyright (c) 2014- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#pragma once

#include <functional>
#include <memory>

#include "ui/ui_screen.h"
#include "ui/viewgroup.h"
#include "UI/MiscScreens.h"
#include "UI/TextureUtil.h"

class GamePauseScreen : public UIDialogScreenWithGameBackground {
public:
	GamePauseScreen(const std::string &filename) : UIDialogScreenWithGameBackground(filename), gamePath_(filename) {}
	virtual ~GamePauseScreen();

	virtual void dialogFinished(const Screen *dialog, DialogResult dr) override;

protected:
	virtual void CreateViews() override;
	virtual void update() override;
	void CallbackDeleteConfig(bool yes);

private:
	PUI::EventReturn OnGameSettings(PUI::EventParams &e);
	PUI::EventReturn OnExitToMenu(PUI::EventParams &e);
	PUI::EventReturn OnReportFeedback(PUI::EventParams &e);

	PUI::EventReturn OnRewind(PUI::EventParams &e);

	PUI::EventReturn OnScreenshotClicked(PUI::EventParams &e);
	PUI::EventReturn OnCwCheat(PUI::EventParams &e);

	PUI::EventReturn OnCreateConfig(PUI::EventParams &e);
	PUI::EventReturn OnDeleteConfig(PUI::EventParams &e);

	PUI::EventReturn OnSwitchUMD(PUI::EventParams &e);
	PUI::EventReturn OnState(PUI::EventParams &e);

	// hack
	bool finishNextFrame_ = false;
	std::string gamePath_;
};

class PrioritizedWorkQueue;

// AsyncImageFileView loads a texture from a file, and reloads it as necessary.
// TODO: Actually make async, doh.
class AsyncImageFileView : public PUI::Clickable {
public:
	AsyncImageFileView(const std::string &filename, PUI::ImageSizeMode sizeMode, PrioritizedWorkQueue *wq, PUI::LayoutParams *layoutParams = 0);
	~AsyncImageFileView();

	void GetContentDimensionsBySpec(const UIContext &dc, PUI::MeasureSpec horiz, PUI::MeasureSpec vert, float &w, float &h) const override;
	void Draw(UIContext &dc) override;

	void DeviceLost() override;
	void DeviceRestored(Draw::DrawContext *draw) override;

	void SetFilename(std::string filename);
	void SetColor(uint32_t color) { color_ = color; }
	void SetOverlayText(std::string text) { text_ = text; }
	void SetFixedSize(float fixW, float fixH) { fixedSizeW_ = fixW; fixedSizeH_ = fixH; }
	void SetCanBeFocused(bool can) { canFocus_ = can; }

	bool CanBeFocused() const override { return canFocus_; }

	const std::string &GetFilename() const { return filename_; }

private:
	bool canFocus_;
	std::string filename_;
	std::string text_;
	uint32_t color_;
	PUI::ImageSizeMode sizeMode_;

	std::unique_ptr<ManagedTexture> texture_;
	bool textureFailed_;
	float fixedSizeW_;
	float fixedSizeH_;
};
