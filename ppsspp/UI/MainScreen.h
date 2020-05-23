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

#include "file/path.h"
#include "ui/ui_screen.h"
#include "ui/viewgroup.h"
#include "UI/MiscScreens.h"

class GameBrowser : public PUI::LinearLayout {
public:
	GameBrowser(std::string path, bool allowBrowsing, bool *gridStyle_, std::string lastText, std::string lastLink, int flags = 0, PUI::LayoutParams *layoutParams = 0);

	PUI::Event OnChoice;
	PUI::Event OnHoldChoice;
	PUI::Event OnHighlight;

	PUI::Choice *HomebrewStoreButton() { return homebrewStoreButton_; }

	void FocusGame(const std::string &gamePath);
	void SetPath(const std::string &path);

protected:
	virtual bool DisplayTopBar();
	virtual bool HasSpecialFiles(std::vector<std::string> &filenames);

	void Refresh();

private:
	bool IsCurrentPathPinned();
	const std::vector<std::string> GetPinnedPaths();
	const std::string GetBaseName(const std::string &path);

	PUI::EventReturn GameButtonClick(PUI::EventParams &e);
	PUI::EventReturn GameButtonHoldClick(PUI::EventParams &e);
	PUI::EventReturn GameButtonHighlight(PUI::EventParams &e);
	PUI::EventReturn NavigateClick(PUI::EventParams &e);
	PUI::EventReturn LayoutChange(PUI::EventParams &e);
	PUI::EventReturn LastClick(PUI::EventParams &e);
	PUI::EventReturn HomeClick(PUI::EventParams &e);
	PUI::EventReturn PinToggleClick(PUI::EventParams &e);

	PUI::ViewGroup *gameList_;
	PathBrowser path_;
	bool *gridStyle_;
	bool allowBrowsing_;
	std::string lastText_;
	std::string lastLink_;
	int flags_;
	PUI::Choice *homebrewStoreButton_;
	std::string focusGamePath_;
};

class RemoteISOBrowseScreen;

class MainScreen : public UIScreenWithBackground {
public:
	MainScreen();
	~MainScreen();

	bool isTopLevel() const override { return true; }

	// Horrible hack to show the demos & homebrew tab after having installed a game from a zip file.
	static bool showHomebrewTab;

protected:
	void CreateViews() override;
	void DrawBackground(UIContext &dc) override;
	void update() override;
	void sendMessage(const char *message, const char *value) override;
	void dialogFinished(const Screen *dialog, DialogResult result) override;

	bool UseVerticalLayout() const;
	bool DrawBackgroundFor(UIContext &dc, const std::string &gamePath, float progress);

	PUI::EventReturn OnGameSelected(PUI::EventParams &e);
	PUI::EventReturn OnGameSelectedInstant(PUI::EventParams &e);
	PUI::EventReturn OnGameHighlight(PUI::EventParams &e);
	// Event handlers
	PUI::EventReturn OnLoadFile(PUI::EventParams &e);
	PUI::EventReturn OnGameSettings(PUI::EventParams &e);
	PUI::EventReturn OnRecentChange(PUI::EventParams &e);
	PUI::EventReturn OnCredits(PUI::EventParams &e);
	PUI::EventReturn OnSupport(PUI::EventParams &e);
	PUI::EventReturn OnPPSSPPOrg(PUI::EventParams &e);
	PUI::EventReturn OnForums(PUI::EventParams &e);
	PUI::EventReturn OnExit(PUI::EventParams &e);
	PUI::EventReturn OnDownloadUpgrade(PUI::EventParams &e);
	PUI::EventReturn OnDismissUpgrade(PUI::EventParams &e);
	PUI::EventReturn OnHomebrewStore(PUI::EventParams &e);
	PUI::EventReturn OnAllowStorage(PUI::EventParams &e);

	PUI::LinearLayout *upgradeBar_;
	PUI::TabHolder *tabHolder_;

	std::string restoreFocusGamePath_;
	std::vector<GameBrowser *> gameBrowsers_;

	std::string highlightedGamePath_;
	std::string prevHighlightedGamePath_;
	float highlightProgress_;
	float prevHighlightProgress_;
	bool backFromStore_;
	bool lockBackgroundAudio_;
	bool lastVertical_;
	bool confirmedTemporary_ = false;

	friend class RemoteISOBrowseScreen;
};

class UmdReplaceScreen : public UIDialogScreenWithBackground {
public:
	UmdReplaceScreen() {}

protected:
	void CreateViews() override;
	void update() override;
	//virtual void sendMessage(const char *message, const char *value);

private:
	PUI::EventReturn OnGameSelected(PUI::EventParams &e);
	PUI::EventReturn OnGameSelectedInstant(PUI::EventParams &e);

	PUI::EventReturn OnCancel(PUI::EventParams &e);
	PUI::EventReturn OnGameSettings(PUI::EventParams &e);
};
