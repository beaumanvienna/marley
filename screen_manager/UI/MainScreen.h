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

#include "Common/UI/UIScreen.h"
#include "Common/UI/ViewGroup.h"
#include "UI/MiscScreens.h"
#include "Common/File/PathBrowser.h"

enum GameBrowserFlags {
	FLAG_HOMEBREWSTOREBUTTON = 1
};

enum class BrowseFlags {
	NONE = 0,
	NAVIGATE = 1,
	ARCHIVES = 2,
	PIN = 4,
	HOMEBREW_STORE = 8,
	STANDARD = 1 | 2 | 4,
};
ENUM_CLASS_BITOPS(BrowseFlags);

class SCREEN_GameBrowser : public SCREEN_UI::LinearLayout {
public:
	SCREEN_GameBrowser(std::string path, BrowseFlags browseFlags, bool *gridStyle, SCREEN_ScreenManager *screenManager, std::string lastText, std::string lastLink, SCREEN_UI::LayoutParams *layoutParams = nullptr);

	SCREEN_UI::Event OnChoice;
	SCREEN_UI::Event OnHoldChoice;
	SCREEN_UI::Event OnHighlight;

	void FocusGame(const std::string &gamePath);
	void SetPath(const std::string &path);
	void Draw(SCREEN_UIContext &dc) override;
	void Update() override;

protected:
	virtual bool DisplayTopBar();
	virtual bool HasSpecialFiles(std::vector<std::string> &filenames);

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
	SCREEN_UI::EventReturn LastClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn HomeClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn PinToggleClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn GridSettingsClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnRecentClear(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnHomebrewStore(SCREEN_UI::EventParams &e);

	SCREEN_UI::ViewGroup *gameList_ = nullptr;
	SCREEN_PathBrowser path_;
	bool *gridStyle_ = nullptr;
	BrowseFlags browseFlags_;
	std::string lastText_;
	std::string lastLink_;
	std::string focusGamePath_;
	bool listingPending_ = false;
	float lastScale_ = 1.0f;
	bool lastLayoutWasGrid_ = true;
	SCREEN_ScreenManager *screenManager_;
};

class RemoteISOBrowseScreen;

class SCREEN_MainScreen : public SCREEN_UIScreenWithBackground {
public:
	SCREEN_MainScreen();
	~SCREEN_MainScreen();

	bool isTopLevel() const override { return true; }

	// Horrible hack to show the demos & homebrew tab after having installed a game from a zip file.
	static bool showHomebrewTab;

protected:
	void CreateViews() override;
	void DrawBackground(SCREEN_UIContext &dc) override;
	void update() override;
	void sendMessage(const char *message, const char *value) override;
	void dialogFinished(const SCREEN_Screen *dialog, DialogResult result) override;

	bool UseVerticalLayout() const;
	bool DrawBackgroundFor(SCREEN_UIContext &dc, const std::string &gamePath, float progress);

	SCREEN_UI::EventReturn OnGameSelected(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnGameSelectedInstant(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnGameHighlight(SCREEN_UI::EventParams &e);
	// Event handlers
	SCREEN_UI::EventReturn OnLoadFile(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnGameSettings(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnCredits(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnSupport(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnPPSSPPOrg(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnForums(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnExit(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnDownloadUpgrade(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnDismissUpgrade(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnAllowStorage(SCREEN_UI::EventParams &e);

	SCREEN_UI::LinearLayout *upgradeBar_ = nullptr;
	SCREEN_UI::TabHolder *tabHolder_ = nullptr;

	std::string restoreFocusGamePath_;
	std::vector<SCREEN_GameBrowser *> gameBrowsers_;

	std::string highlightedGamePath_;
	std::string prevHighlightedGamePath_;
	float highlightProgress_ = 0.0f;
	float prevHighlightProgress_ = 0.0f;
	bool backFromStore_ = false;
	bool lockBackgroundAudio_ = false;
	bool lastVertical_;
	bool confirmedTemporary_ = false;

	friend class RemoteISOBrowseScreen;
};

class SCREEN_UmdReplaceScreen : public SCREEN_UIDialogScreenWithBackground {
public:
	SCREEN_UmdReplaceScreen() {}

protected:
	void CreateViews() override;
	void update() override;
	//virtual void sendMessage(const char *message, const char *value);

private:
	SCREEN_UI::EventReturn OnGameSelected(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnGameSelectedInstant(SCREEN_UI::EventParams &e);

	SCREEN_UI::EventReturn OnCancel(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnGameSettings(SCREEN_UI::EventParams &e);
};

class GridSettingsScreen : public SCREEN_PopupScreen {
public:
	GridSettingsScreen(std::string label) : SCREEN_PopupScreen(label) {}
	void CreatePopupContents(SCREEN_UI::ViewGroup *parent) override;
	SCREEN_UI::Event OnRecentChanged;

private:
	SCREEN_UI::EventReturn GridPlusClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn GridMinusClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnRecentClearClick(SCREEN_UI::EventParams &e);
	const float MAX_GAME_GRID_SCALE = 3.0f;
	const float MIN_GAME_GRID_SCALE = 0.8f;
};
