// Copyright (c) 2013-2020 PPSSPP project
// Copyright (c) 2020 Marley project

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

#include "ppsspp_config.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include "Common/UI/UIScreen.h"
#include "Common/File/PathBrowser.h"
#include "UI/MiscScreens.h"
#include "UI/SettingsScreen.h"

class SCREEN_MainInfoMessage;
class SCREEN_GameBrowser;

// Per-game settings screen - enables you to configure graphic options, control options, etc
// per game.
class SCREEN_MainScreen : public SCREEN_UIDialogScreenWithBackground {
public:
    SCREEN_MainScreen();
    virtual ~SCREEN_MainScreen();
	void update() override;
	void onFinish(DialogResult result) override;
	std::string tag() const override { return "settings"; }
    void DrawBackground(SCREEN_UIContext &dc) override;

protected:
	void CreateViews() override;
	void CallbackRestoreDefaults(bool yes);
	void CallbackRenderingBackend(bool yes);
	void CallbackRenderingDevice(bool yes);
	void CallbackInflightFrames(bool yes);
	bool UseVerticalLayout() const;
    
    // game browser
    SCREEN_GameBrowser *searchDirBrowser;
   	SCREEN_UI::EventReturn OnGameSelected(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnGameSelectedInstant(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnGameHighlight(SCREEN_UI::EventParams &e);


private:
    
	void TriggerRestart(const char *why);

	bool lastVertical_;
	SCREEN_MainInfoMessage *settingInfo_;

	// Event handlers
    
    

};

class SCREEN_MainInfoMessage : public SCREEN_UI::LinearLayout {
public:
	SCREEN_MainInfoMessage(int align, SCREEN_UI::AnchorLayoutParams *lp);

	void SetBottomCutoff(float y) {
		cutOffY_ = y;
	}
	void Show(const std::string &text, SCREEN_UI::View *refView = nullptr);

	void Draw(SCREEN_UIContext &dc);

private:
	SCREEN_UI::TextView *text_ = nullptr;
	double timeShown_ = 0.0;
	float cutOffY_;
};

class SCREEN_GameBrowser : public SCREEN_UI::LinearLayout {
public:
	SCREEN_GameBrowser(std::string path, SCREEN_BrowseFlags browseFlags, bool *gridStyle, SCREEN_ScreenManager *screenManager, std::string lastText, std::string lastLink, SCREEN_UI::LayoutParams *layoutParams = nullptr);
    ~SCREEN_GameBrowser();
	SCREEN_UI::Event OnChoice;
	SCREEN_UI::Event OnHoldChoice;
	SCREEN_UI::Event OnHighlight;

	void FocusGame(const std::string &gamePath);
	void SetPath(const std::string &path);
    std::string GetPath();
	void Draw(SCREEN_UIContext &dc) override;
	void Update() override;

protected:
	virtual bool DisplayTopBar();

	void Refresh();

private:
	bool IsCurrentPathPinned();
	const std::vector<std::string> GetPinnedPaths();
	const std::string GetBaseName(const std::string &path);

	SCREEN_UI::EventReturn GameButtonClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn GameButtonHoldClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn GameButtonHighlight(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn NavigateClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn LayoutChange(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn HomeClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn GridSettingsClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnRecentClear(SCREEN_UI::EventParams &e);

	SCREEN_UI::ViewGroup *gameList_ = nullptr;
	SCREEN_PathBrowser path_;
	bool *gridStyle_ = nullptr;
	SCREEN_BrowseFlags browseFlags_;
	std::string lastText_;
	std::string lastLink_;
	std::string focusGamePath_;
	bool listingPending_ = false;
	float lastScale_ = 1.0f;
	bool lastLayoutWasGrid_ = true;
	SCREEN_ScreenManager *screenManager_;
};


class SCREEN_GridMainScreen : public SCREEN_PopupScreen {
public:
	SCREEN_GridMainScreen(std::string label) : SCREEN_PopupScreen(label) {}
	void CreatePopupContents(SCREEN_UI::ViewGroup *parent) override;
	SCREEN_UI::Event OnRecentChanged;

private:
	SCREEN_UI::EventReturn GridPlusClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn GridMinusClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnRecentClearClick(SCREEN_UI::EventParams &e);
	const float MAX_GAME_GRID_SCALE = 3.0f;
	const float MIN_GAME_GRID_SCALE = 0.8f;
};
