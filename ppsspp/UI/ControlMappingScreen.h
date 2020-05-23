// Copyright (c) 2013- PPSSPP Project.

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
#include <vector>
#include <mutex>

#include "i18n/i18n.h"
#include "ui/view.h"
#include "ui/ui_screen.h"

#include "UI/MiscScreens.h"

class ControlMapper;

class ControlMappingScreen : public UIDialogScreenWithBackground {
public:
	ControlMappingScreen() {}
	void KeyMapped(int pspkey);  // Notification to let us refocus the same one after recreating views.
	std::string tag() const override { return "control mapping"; }

protected:
	virtual void CreateViews() override;
private:
	PUI::EventReturn OnDefaultMapping(PUI::EventParams &params);
	PUI::EventReturn OnClearMapping(PUI::EventParams &params);
	PUI::EventReturn OnAutoConfigure(PUI::EventParams &params);
	PUI::EventReturn OnTestAnalogs(PUI::EventParams &params);

	virtual void dialogFinished(const Screen *dialog, DialogResult result) override;

	PUI::ScrollView *rightScroll_;
	std::vector<ControlMapper *> mappers_;
};

class KeyMappingNewKeyDialog : public PopupScreen {
public:
	explicit KeyMappingNewKeyDialog(int btn, bool replace, std::function<void(KeyDef)> callback, I18NCategory *i18n)
		: PopupScreen(i18n->T("Map Key"), "Cancel", ""), callback_(callback), mapped_(false) {
		pspBtn_ = btn;
	}

	virtual bool key(const KeyInput &key) override;
	virtual bool axis(const AxisInput &axis) override;

protected:
	void CreatePopupContents(PUI::ViewGroup *parent) override;

	virtual bool FillVertical() const override { return false; }
	virtual bool ShowButtons() const override { return true; }
	virtual void OnCompleted(DialogResult result) override {}

private:
	int pspBtn_;
	std::function<void(KeyDef)> callback_;
	bool mapped_;  // Prevent double registrations
};

class KeyMappingNewMouseKeyDialog : public PopupScreen {
public:
	explicit KeyMappingNewMouseKeyDialog(int btn, bool replace, std::function<void(KeyDef)> callback, I18NCategory *i18n)
		: PopupScreen(i18n->T("Map Mouse"), "", ""), callback_(callback), mapped_(false) {
		pspBtn_ = btn;
	}

	virtual bool key(const KeyInput &key) override;
	virtual bool axis(const AxisInput &axis) override;

protected:
	void CreatePopupContents(PUI::ViewGroup *parent) override;

	virtual bool FillVertical() const override { return false; }
	virtual bool ShowButtons() const override { return true; }
	virtual void OnCompleted(DialogResult result) override {}

private:
	int pspBtn_;
	std::function<void(KeyDef)> callback_;
	bool mapped_;  // Prevent double registrations
};

class AnalogTestScreen : public UIDialogScreenWithBackground {
public:
	AnalogTestScreen() {}

	bool key(const KeyInput &key) override;
	bool axis(const AxisInput &axis) override;

protected:
	virtual void CreateViews() override;

	PUI::TextView *lastKeyEvent_ = nullptr;
	PUI::TextView *lastLastKeyEvent_ = nullptr;
};

class TouchTestScreen : public UIDialogScreenWithBackground {
public:
	TouchTestScreen() {
		for (int i = 0; i < MAX_TOUCH_POINTS; i++) {
			touches_[i].id = -1;
		}
	}

	bool touch(const TouchInput &touch) override;
	void render() override;

protected:
	struct TrackedTouch {
		int id;
		float x;
		float y;
	};
	enum {
		MAX_TOUCH_POINTS = 10,
	};
	TrackedTouch touches_[MAX_TOUCH_POINTS]{};

	virtual void CreateViews() override;
	PUI::EventReturn OnImmersiveModeChange(PUI::EventParams &e);
};
