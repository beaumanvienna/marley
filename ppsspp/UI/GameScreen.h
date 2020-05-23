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

#include "UI/MiscScreens.h"
#include "ui/ui_screen.h"

// Game screen: Allows you to start a game, delete saves, delete the game,
// set game specific settings, etc.

// Uses GameInfoCache heavily to implement the functionality.
// Should possibly merge this with the PauseScreen.

class GameScreen : public UIDialogScreenWithGameBackground {
public:
	GameScreen(const std::string &gamePath);
	~GameScreen();

	void render() override;

	std::string tag() const override { return "game"; }

protected:
	void CreateViews() override;
	void CallbackDeleteConfig(bool yes);
	void CallbackDeleteSaveData(bool yes);
	void CallbackDeleteGame(bool yes);
	bool isRecentGame(const std::string &gamePath);

private:
	PUI::Choice *AddOtherChoice(PUI::Choice *choice);

	// Event handlers
	PUI::EventReturn OnPlay(PUI::EventParams &e);
	PUI::EventReturn OnGameSettings(PUI::EventParams &e);
	PUI::EventReturn OnDeleteSaveData(PUI::EventParams &e);
	PUI::EventReturn OnDeleteGame(PUI::EventParams &e);
	PUI::EventReturn OnSwitchBack(PUI::EventParams &e);
	PUI::EventReturn OnCreateShortcut(PUI::EventParams &e);
	PUI::EventReturn OnRemoveFromRecent(PUI::EventParams &e);
	PUI::EventReturn OnShowInFolder(PUI::EventParams &e);
	PUI::EventReturn OnCreateConfig(PUI::EventParams &e);
	PUI::EventReturn OnDeleteConfig(PUI::EventParams &e);
	PUI::EventReturn OnCwCheat(PUI::EventParams &e);
	PUI::EventReturn OnSetBackground(PUI::EventParams &e);

	// As we load metadata in the background, we need to be able to update these after the fact.
	PUI::TextView *tvTitle_;
	PUI::TextView *tvGameSize_;
	PUI::TextView *tvSaveDataSize_;
	PUI::TextView *tvInstallDataSize_;
	PUI::TextView *tvRegion_;

	PUI::Choice *btnGameSettings_;
	PUI::Choice *btnCreateGameConfig_;
	PUI::Choice *btnDeleteGameConfig_;
	PUI::Choice *btnDeleteSaveData_;
	PUI::Choice *btnSetBackground_;
	std::vector<PUI::Choice *> otherChoices_;
	std::vector<std::string> saveDirs;
};
